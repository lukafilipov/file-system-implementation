#pragma once
#include "RootDir.h"

RootDir::RootDir(PartWrap* p, ClusterNo n)
{
	myPW = p;
	rootNo = n;
	numOfFiles = 0;
	curNum = 0;
	rootCluster = new char[2048];
	curCluster = new char[2048];
	myPW->part->readCluster(rootNo, rootCluster);
}

RootDir::~RootDir()
{
	myPW->part->writeCluster(rootNo, rootCluster);
	if (curNum != 0) myPW->part->writeCluster(curNum, curCluster);
	delete rootCluster;
	delete curCluster;
}

KernelFile* RootDir::find(char* name, char* ext, char mode)
{
	char* cluster = new char[2048];
	for (int i = 0; i < 256; i++)
	{

		ClusterNo num = ToLong(rootCluster, i * 4); 
		if (num == 0) return nullptr;
		if (num == curNum) cluster = curCluster;
		else myPW->part->readCluster(num, cluster);
		for (int j = 0; j < 102; j++)
		{
			if (cluster[j * 20] == 0) return nullptr;
			if ((strncmp(cluster + j * 20, name, 8) == 0) && (strncmp(cluster + j*20 + 8, ext, 3) == 0))
			{
				ClusterNo size = ToLong(cluster, j * 20 + 0x10); 
				ClusterNo num = ToLong(cluster, j * 20 + 0x0c); 
				numOfLastFound = i*102 + j;
				return new KernelFile(myPW, size, num, name, ext, i*102 + j, mode);
			}
		}
	}
	return nullptr;

}

char RootDir::readRootDir(char part, EntryNum n, Directory & d)
{
	char* cluster = new char[2048];
	char numRead = 0;
	for (unsigned i = n/102; i <= n/102 + 1; i++)
	{
		ClusterNo num = ToLong(rootCluster, i * 4);
		if (num == 0) return numRead;
		if (num == curNum) cluster = curCluster;
		else myPW->part->readCluster(num, cluster);
		for (int j = n % 102; j < 102; j++)
		{
			if (cluster[j * 20] == 0) return numRead;
			if (numRead == 64) return 65;
			for (int k = 0; k < 8; k++) d[numRead].name[k] = cluster[j * 20 + k];
			for (int k = 0; k < 3; k++) d[numRead].ext[k] = cluster[j * 20 + 8 + k];
			d[numRead].reserved = 0;
			d[numRead].indexCluster = ToLong(cluster, j * 20 + 0x0c);
			d[numRead].size = ToLong(cluster, j * 20 + 0x10);
			numRead++;
		}
	}
	return 0;
}

KernelFile* RootDir::makeNewFile(char *name, char *ext)
{
	if (numOfFiles % 102 == 0)
	{
		if (curNum != 0) myPW->part->writeCluster(curNum, curCluster);
		std::fill(curCluster, curCluster + 2048, 0);
		if (myPW->freeSegments.empty()) return 0;
		curNum = myPW->getFirstFree();
		myPW->updateBitVector(curNum, 1);
		ToChar(curNum, rootCluster, (numOfFiles / 102) * 4);

	}
	memcpy(curCluster + (numOfFiles % 102) * 20, name, 8);
	memcpy(curCluster + (numOfFiles % 102) * 20 + 8, ext, 3);
	if (myPW->freeSegments.empty()) return 0;
	long index1 = myPW->getFirstFree();
	myPW->updateBitVector(index1 , 1);
	ToChar(index1, curCluster, (numOfFiles % 102) * 20 + 0x0c);
	numOfFiles++;
	return new KernelFile(myPW, 0, index1, name, ext, numOfFiles-1, 'w');
}

char RootDir::deleteFile(char * name, char * ext)
{
	unique_lock<mutex> lock(myPW->partMutex);
	if (myPW->findOpenFile(name, ext) != nullptr)
	{
		lock.unlock();
		return 0;
	}
	KernelFile* kf = find(name, ext, 'd');
	if (kf == nullptr) return 0;
	char *clusterFile;
	ClusterNo numFileC;
	int b = numOfLastFound;
	bool c = 0;
	if ((b % 102 == 0) && (b !=0)) c = 1;
	numFileC = ToLong(rootCluster, (b / 102 - c) * 4);
	if (numOfFiles != numOfLastFound)
	{
		if (curNum == numFileC) clusterFile = curCluster;
		else { clusterFile = new char[2048];  myPW->part->readCluster(numFileC, clusterFile); }
		for (int i = 0; i < 20; i++)
		{
			clusterFile[(b % 102) * 20 + i] = curCluster[numOfFiles * 20 - 20 + i];
			curCluster[numOfFiles * 20 - 20 + i] = 0;
		}
	}
	numOfFiles--;
	if ((numOfFiles % 102) == 0)
	{
		myPW->updateBitVector(curNum, 0);
		for (int i = 0; i < 4; i++) rootCluster[(numOfFiles / 102) * 4 + i] = 0;
		if (numOfFiles != 0)
		{
			curNum = ToLong(rootCluster, (numOfFiles / 102) * 4 - 4);
			myPW->part->readCluster(curNum, curCluster);
		}
		else
		{
			curNum = 0;
			curCluster = nullptr;
		}
	}
	else
	{
		for (int i = 0; i < 20; i++)
		{
			curCluster[(numOfFiles % 102) * 20 + i] = 0;
		}
	}
	lock.unlock();
	kf->seek(0);
	kf->truncate();;
	myPW->updateBitVector(kf->indNum, 0);
	return 1;
}

void RootDir::updateFileSize(ClusterNo fileNo, ClusterNo fileSize)
{
	ClusterNo clusterNo = ToLong(rootCluster, (fileNo / 102) * 4);
	char *cluster;
	if (clusterNo == curNum)
	{
		cluster = curCluster;
		ToChar(fileSize, curCluster, (fileNo % 102) * 20 + 0x10);
	}
	else
	{
		cluster = new char[2048];
		myPW->part->readCluster(clusterNo, cluster);
		ToChar(fileSize, cluster, (fileNo % 102) * 20 + 0x10);
		myPW->part->writeCluster(clusterNo, cluster);
		delete cluster;
	}	
}
