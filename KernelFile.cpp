#pragma once
#include "KernelFile.h"
#include "string.h"
#include <iostream>

using namespace std;

bool operator==(OpenFileListElem &a, const OpenFileListElem &b)
{
	return (a.indNum == b.indNum);
}

KernelFile::KernelFile(PartWrap *PW, ClusterNo siz, ClusterNo num, char* nam, char* ex, long fileN, char mod)
{
	pos = 0;
	lastindex1 = lastindex2 = 0;
	size = siz;
	cachePos = -2048;
	cacheNum = 0;
	oldpos = ~0;
	indNum = num;
	dirty = false;
	in2d = false;
	in2num = 0;
	name = nam;
	ext = ex;
	fileNo = fileN;
	mode = mod;
	myPW = PW;
	index1 = new char[2048];
	index2 = new char[2048]; //kad je ovde bio if dolazilo je do greske sa najvecom slikom
	cacheClust = new char[2048];
	myPW->part->readCluster(indNum, index1);
	if (mod == 'a')
	{
		if (size <= 256 * 2048)
		{
			lastindex1 = (size / 2048 + 1) * 4; 
		}
		else
		{
			lastindex1 = ((size - 256 * 2048) / (512 * 2048) + 256 + 1) * 4;
			lastindex2 = (((size - 256 * 2048) / 2048) % 512 + 1) * 4;
		}

	}
}
KernelFile::~KernelFile()
{
	myEntry->numOpen--;
	myPW->numOpenFiles--;
	if (mode != 'r') myPW->part->writeCluster(indNum, index1);
	if ((mode != 'r') && (in2num != 0)) myPW->part->writeCluster(in2num, index2);
	if ((mode != 'r') && (dirty != 0)) myPW->part->writeCluster(cacheNum, cacheClust);
	if ((myEntry->numOpen == 0) && (myEntry->numWaiting > 0))
	{
		myEntry->size = size;
		myEntry->con->notify_all();
	}
	else
	if ((myEntry->numOpen == 0) && (myEntry->numWaiting == 0))
	{
		myPW->fileListHead.remove(myEntry);
		myPW->rootDir->updateFileSize(fileNo, size);
		if (myPW->FUrequest == 1)
		{
			unique_lock<mutex> lock(myPW->partMutex);
			myPW->partCon.notify_all();
			lock.unlock();
		}
	}
	delete[] index1;
	delete[] index2;
	delete[] cacheClust;
	
} 
char KernelFile::write(BytesCnt cnt, char *buffer)
{
	if (mode == 'r') return 0;
	//char *block = new char[2048];
	if(pos%2048!=0)
	{
		//myPW->part->readCluster(findCluster(), block);
		updateCache();
		dirty = 1;
		int a = cnt < (2048 - pos % 2048) ? cnt : (2048 - pos % 2048);
		memcpy(cacheClust + pos % 2048, buffer, a);
		//myPW->part->writeCluster(findCluster(), block);
		pos += a; buffer += a; cnt -= a;
		if (pos > size) size = pos;
	}
	while (cnt >=2048)
	{
		if (size == pos) if (allocateCluster() == 0) return 0;
		memcpy(cacheClust, buffer, 2048);
		buffer += 2048;
		myPW->part->writeCluster(findCluster(), cacheClust);
		pos += 2048;
		cnt -= 2048;
		if (pos > size) size = pos;
	}
	if (cnt>0)
	{
		if ((size == pos) && (size%2048 == 0)) if (allocateCluster() == 0) return 0;
		updateCache();
		dirty = 1;
	    //myPW->part->readCluster(findCluster(), block);
		memcpy(cacheClust, buffer, cnt);
		//myPW->part->writeCluster(findCluster(), block);
		pos += cnt;
		if (pos > size) size = pos;
	}
	//delete[] block;
	return 1;
}
BytesCnt KernelFile::read(BytesCnt cnt, char *buffer)
{
	int read = 0;
	//char *block = new char[2048];
	if (cnt <= (2048 - pos % 2048))
	{
		//myPW->part->readCluster(findCluster(), block);
		updateCache();
		if (size - pos > cnt)
		{
			memcpy(buffer, cacheClust + pos % 2048, cnt);
			pos += cnt;
			//delete[] block;
			return cnt;
		}
		else
		{
			memcpy(buffer, cacheClust + pos % 2048, size-pos);
			int cur = pos;
			pos = size;
			//delete[] block;
			return (size - cur);
		}
	}
	else
	{
		//myPW->part->readCluster(findCluster(), block);
		updateCache();
		if (size - pos > 2048 - pos % 2048)
		{
			memcpy(buffer, cacheClust + pos % 2048, 2048 - pos % 2048);
			read += 2048 - pos % 2048;
		}
		else
		{
			memcpy(buffer, cacheClust + pos % 2048, size - pos);
			pos = size;
			//delete[] block;
			return (size - pos);
		}
		buffer += 2048 - pos % 2048; //sta ovo uradi?
		pos += 2048 - pos % 2048;
		cnt -= 2048 - pos % 2048;
		while (cnt >= 2048)
		{
			//if (pos / 2048 < 256)
			//myPW->part->readCluster(findCluster(), block);
			updateCache();
			if (size - pos >= 2048)
			{
				memcpy(buffer, cacheClust, 2048);
				read += 2048;
				buffer += 2048;
				pos += 2048;
				cnt -= 2048;
			}
			else
			{
				memcpy(buffer, cacheClust, size-pos);
				read += size - pos;
				pos = size;
				//delete[] block;
				return read;
			}
		}
		if (cnt > 0)
		{
			//myPW->part->readCluster(findCluster(), block);
			updateCache();
			if (size - pos > cnt)
			{
				memcpy(buffer, cacheClust, cnt);
				read += cnt;
				pos += cnt;
			}
			else
			{
				memcpy(buffer, cacheClust, size - pos);
				read += size - pos;
				pos = size;
			}
		}
		//delete[] block;
		return read;
	}

}
char KernelFile::seek(BytesCnt cnt)
{
	if (cnt > size) return 0;
	pos = cnt;
	return 1;
}
BytesCnt KernelFile::filePos()
{
	return pos;
}
char KernelFile::eof()
{
	if (pos == size) return 2;
	else return 0;
}
BytesCnt KernelFile::getFileSize()
{
	return size;
}
char KernelFile::allocateCluster()
{
	if (myPW->freeSegments.empty()) return 0;
	if (size == MAXSIZE) return 0;
	if (size < 256 * 2048) //ako je indexiranje u jednom nivou
	{
		long free = myPW->getFirstFree();
		myPW->updateBitVector(free, 1);
		ToChar(free, index1, lastindex1);
		lastindex1 += 4;
	}
	else  //ako je u 2 nivoa
	{ 
		if ((size - 256 * 2048) % (512 * 2048) == 0) 
		{
			long free = myPW->getFirstFree();
			myPW->updateBitVector(free, 1);
			ToChar(free, index1, lastindex1);
			lastindex1 += 4;
			if (in2d == true)
			{
				myPW->part->writeCluster(in2num, index2);  //vracanje *index2 na disk
				in2num = free;
				lastindex2 = 0;
				std::fill(index2, index2 + 2048, 0);
			}
			else
			{
				index2 = new char[2048];
				in2num = free;
			}
		}
		if (myPW->freeSegments.empty()) return 0;
		long free = myPW->getFirstFree();
		myPW->updateBitVector(free, 1);
		in2d = true;
		ToChar(free, index2, lastindex2);
		lastindex2 += 4;
	}
	return 1;
}
ClusterNo KernelFile::findCluster()
{
	if (pos < 256 * 2048)
	{
		int place = (pos / 2048) * 4;
		ClusterNo num = ToLong(index1, place); 
		return num;
	}
	else
	{
		if ((pos - 256 * 2048) / (512 * 2048) != (oldpos - 256 * 2048) / (512 * 2048))
		{
			if (in2d)
			{
				myPW->part->writeCluster(in2num, index2);
				in2d = 0;
			}
			int num = ((pos - 256 * 2048) / (512 * 2048) + 256) * 4; //Proveriti ovo
			in2num = ToLong(index1, num); 
			myPW->part->readCluster(in2num, index2);
			oldpos = pos;

		}
		int place = (((pos - 256 * 2048) / 2048) % 512) * 4;    //(pos / 2048) * 4;
		ClusterNo num = ToLong(index2, place); 
		return num;
	}
}
char KernelFile::truncate()
{
	unsigned long end = size;
	size = pos;
	int origin2 = 0;
	if ((pos > 256 * 2048) && ((pos - 256 * 2048) % (512 * 2048) != 0))
	{
		origin2 = in2num;
	}
	if (pos % 2048 != 0)
	{
		pos += 2048 - pos % 2048;
	}
	while (pos < end)
	{
		ClusterNo num = findCluster();
		myPW->updateBitVector(num, 0);
		/* if ((in2num != origin2) && (in2num!=0) && ((pos == end) || ((pos - 256 * 2048 + 1) % (512 * 2048) == 0)))
		{
			myPW->updateBitVector(in2num, 0);
		} */
		pos += 2048;
	}
	pos = size;
	return 1;
}

void KernelFile::updateCache()
{
	if ((cachePos / 2048) != (pos / 2048))
	{
		if ((cacheNum != 0) && (dirty == true))
		{
			myPW->part->writeCluster(cacheNum, cacheClust);
		}
		cacheNum = findCluster();
		myPW->part->readCluster(cacheNum, cacheClust);
		cachePos = pos;
	}
}
