#pragma once
#include "PartWrap.h"
#include "file.h"
#include "fs.h"

PartWrap::PartWrap(Partition* p)
{
	part = p;
	numOpenFiles = 0;
	FUrequest = 0;

}
PartWrap::~PartWrap()
{
	delete rootDir;
}
ClusterNo PartWrap::getFirstFree()
{
	FreeSegment *s = &(freeSegments.front());
	if (s->start == s->end) 
	{
		ClusterNo n = s->start;  
		freeSegments.pop_front();
		// delete s ili s==nullptr mozda
		return n;
	}
	s->start++;
	return (s->start - 1);
}

void PartWrap::updateBitVector(ClusterNo numOfCluster, int value)
{
	long div = numOfCluster / (2048 * 8);
	char numBit = numOfCluster % 8;
	int numByte = (numOfCluster % (2048 * 8)) / 8; //Ovo je bilo pre: numOfCluster / 8;
	char* bitVector = new char[2048];
	part->readCluster(div, bitVector);
	if (numOfCluster == 2)
	{
		int b = 3;
	}
	if (value == 1) bitVector[numByte] |= 1 << (7 - numBit);
	else
	{
		bool a = true;
		bitVector[numByte] &= _rotl(0x7F, numBit);
		for (list<FreeSegment>::iterator it = freeSegments.begin(); it != freeSegments.end(); it++)
		{
			if ((numOfCluster + 1) == it->start)
			{
				it->start--;
				a = false;
				break;
			}
			if ((numOfCluster - 1) == it->end)
			{
				it->end++;
				a = false;
				break;
			}
		}
		if (a == true) freeSegments.push_front(FreeSegment(numOfCluster, numOfCluster));
	}
	part->writeCluster(div, bitVector);
	delete[] bitVector;
}

bool PartWrap::checkFormated()
{
	char* name = new char[2048];
	part->readCluster(part->getNumOfClusters()-1, name);
	if (strcmp("Ova particija je formatirana", name) == 0)
	{
		delete[] name;
		return true;
	}
	delete[] name;
	return false;
}

KernelFile * PartWrap::openRead(char * name, char *ext)
{
	unique_lock<mutex> lck(partMutex);
	KernelFile *kf;
	OpenFileListElem* fl = findOpenFile(name, ext);
	if (fl == nullptr)
	{
		kf = rootDir->find(name, ext, 'r');
		if (kf == nullptr)
		{
			lck.unlock();
			return nullptr;
		}
		fl = new OpenFileListElem(kf->size, kf->indNum, kf->fileNo, name, ext, 'r');
		fileListHead.push_front(fl);
	}
	else
	{
		kf = new KernelFile(this, fl->size, fl->indNum, name, ext, fl->fileNo, 'r');
	}
	kf->myEntry = fl;
	lck.unlock();
	unique_lock<mutex> lock(*(fl->mut));
	fl->numWaiting++;
	while ((fl->mode != 'r') && (fl->numOpen>0))
	{
		fl->con->wait(lock);
	}
	kf->size = fl->size;
	kf->myPW->part->readCluster(kf->indNum, kf->index1);
	fl->numWaiting--;
	fl->mode = 'r';
	numOpenFiles++;
	fl->numOpen++;
	lock.unlock();
	return kf;
}

KernelFile * PartWrap::openWrite(char * name, char* ext, char mode)
{
	unique_lock<mutex> lock(partMutex);
	KernelFile* kf;
	OpenFileListElem* fl = findOpenFile(name, ext);
	if ((fl == nullptr) && (mode == 'w'))
	{
	    kf = rootDir->find(name, ext, 'w');
		if (kf != nullptr)
		{		
			fl = new OpenFileListElem(0, kf->indNum, kf->fileNo, name, ext, mode);
			fl->numOpen++;
			numOpenFiles++;
			fileListHead.push_back(fl);
			kf->myEntry = fl;
			lock.unlock();
			kf->seek(0);
			kf->truncate();
			return kf;
		}
		else
		{			
			kf = rootDir->makeNewFile(name, ext);
			fl = new OpenFileListElem(0, kf->indNum, kf->fileNo, name, ext, mode);
			fl->numOpen++;
			numOpenFiles++;
			fileListHead.push_back(fl);
			kf->myEntry = fl;
			lock.unlock();
			return kf;
		}
	}
	else
	if ((fl == nullptr) && (mode == 'a'))
	{
		kf = rootDir->find(name, ext, 'a');
		if (kf != nullptr)
		{
			kf->seek(kf->getFileSize());
		}
		else
		{
			lock.unlock();
			return nullptr;
		}
		fl = new OpenFileListElem(kf->size, kf->indNum, kf->fileNo, name, ext, mode);
		fl->numOpen++;
		numOpenFiles++;
		fileListHead.push_back(fl);
		kf->myEntry = fl;
		lock.unlock();
		return kf;
	}
	else
	if ((fl != nullptr) && (mode == 'a'))
	{
		lock.unlock();
		unique_lock<mutex> lck(*(fl->mut));
		fl->numWaiting++;
		while (fl->numOpen > 0)
		{
			fl->con->wait(lck);
		}
		fl->numWaiting--;
		fl->numOpen++;
		numOpenFiles++;
		kf = new KernelFile(this, fl->size, fl->indNum, name, ext, fl->fileNo, 'a');
		kf->myEntry = fl;
		kf->seek(kf->getFileSize());
		lck.unlock();
		return kf;
	}
	else
	if ((fl != nullptr) && (mode == 'w'))
	{
		lock.unlock();
		unique_lock<mutex> lck(*(fl->mut));
		fl->numWaiting++;
		while (fl->numOpen > 0)
		{
			fl->con->wait(lck);
		}
		fl->numWaiting--;
		fl->numOpen++;
		numOpenFiles++;
		kf = new KernelFile(this, fl->size, fl->indNum, name, ext, fl->fileNo, 'w');
		kf->myEntry = fl;
		kf->seek(0);
		kf->truncate();
		fl->size = 0;
		lck.unlock();
		return kf;
	}	
	return nullptr;
}

OpenFileListElem *PartWrap::findOpenFile(char * name, char *ext)
{
	for (list<OpenFileListElem*>::iterator it = fileListHead.begin(); it != fileListHead.end(); it++)
	{
		if ((strncmp((*it)->name, name, 8) == 0) && (strncmp((*it)->ext, ext, 3) == 0))
		{
			return *it;
		}
	}
	return nullptr;
}
void ToChar(long from, char* buffer, int i)
{
	buffer[i++] = (from >> 24) & 0xff;
	buffer[i++] = (from >> 16) & 0xff;
	buffer[i++] = (from >> 8) & 0xff;
	buffer[i++] = from & 0xff;
}

long ToLong(char* buffer, int i)
{
	return ((buffer[i] & 0xff) << 24) + ((buffer[i + 1] & 0xff) << 16) + ((buffer[i + 2] & 0xff) << 8) + ((buffer[i + 3] & 0xff));
}
