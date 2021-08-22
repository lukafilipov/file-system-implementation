#pragma once
#include "fs.h"
#include "kernelFS.h"
#include <iostream>

PartWrap *KernelFS::pw[26] = { nullptr };

char KernelFS::mount(Partition * partition)
{
	for (int i = 0; i < 26; i++)
	{
		if (pw[i] == nullptr)
		{
			pw[i] = new PartWrap(partition);
			// if (!pw[i]->isFormated()) format('A' + i); // Ovo mozda ne treba 
			unique_lock<mutex> lock(pw[i]->partMutex);
			if (pw[i]->isFormated = pw[i]->checkFormated())
			{
				pw[i]->FUrequest = 1;
				ClusterNo n = pw[i]->part->getNumOfClusters();
				int a, j;
				if (n % (2048 * 8)) a = 1;
				else a = 0;
				ClusterNo rootClusterNum = n / (2048 * 8) + a;
				pw[i]->rootClustNum = rootClusterNum;
				pw[i]->rootDir = new RootDir(pw[i], pw[i]->rootClustNum);
				pw[i]->part->readCluster(rootClusterNum, pw[i]->rootDir->rootCluster);
				char* cluster = new char[2048];
				for (int k = 0; k < 256; k++)
				{
					ClusterNo num = ToLong(pw[i]->rootDir->rootCluster, k * 4);
					if (num == 0) break;
					else pw[i]->part->readCluster(num, cluster);
					for (j = 0; j < 102; j++)
					{
						if (cluster[j * 20] == 0)
						{
							pw[i]->rootDir->curNum = num;
							pw[i]->rootDir->curCluster = cluster;
							pw[i]->rootDir->numOfFiles = k * 102 + j;
							break;
						}
					}
					if (j != 102) break;
				}
				PartWrap::FreeSegment s(rootClusterNum + 1, n - 2);
				pw[i]->freeSegments.push_front(s);
				pw[i]->FUrequest = 0;
				pw[i]->isFormated = 1;				
			}
			lock.unlock();
		    // pw[i]->rootDir = new RootDir(pw[i], pw[i]->rootClustNum);
			return ('A' + i);
		}
	}
	return 0;
}

char KernelFS::unmount(char part)
{
	if (pw[part - 'A'] == nullptr) return 0;
	unique_lock<mutex> lock(pw[part - 'A']->partMutex);
	if (pw[part - 'A']->FUrequest == 1)
	{
		lock.unlock();
		return 0;
	}
	pw[part - 'A']->FUrequest = 1;
	while (pw[part - 'A']->numOpenFiles > 0)
	{
		pw[part - 'A']->partCon.wait(lock);
	}
	lock.unlock();
	delete pw[part - 'A'];
	pw[part - 'A'] = nullptr;
	return 1;
}

char KernelFS::format(char part)
{
	if (pw[part - 'A'] == nullptr) return 0;
	unique_lock<mutex> lock(pw[part - 'A']->partMutex);
	if (pw[part - 'A']->FUrequest == 1)
	{
		lock.unlock();
		return 0;
	}
	pw[part - 'A']->FUrequest = 1;
	while (pw[part - 'A']->numOpenFiles > 0)
	{
		pw[part - 'A']->partCon.wait(lock);
	}
	ClusterNo n = pw[part - 'A']->part->getNumOfClusters();
	char* cluster = new char[2048];
	std::fill(cluster, cluster + 2048, 0);
	std::fill(cluster, cluster + n / (2048 * 8 * 8), 0xFF);
	int a;
	if (n % (2048 * 8)) a = 1; 
	else a = 0; 
	cluster[n / (2048 * 8 * 8)] = 0xFF << (8 - ((n / (2048 * 8)) % 8 + a )); //Proveriti ovo!!! Da li a treba ovde? Izgleda da treba
	ClusterNo rootCluster = n / (2048 * 8) + a; //ovde treba
	cluster[rootCluster / 8] |= 0x01 << (7 - rootCluster % 8);
	pw[part - 'A']->rootClustNum = rootCluster;
	pw[part - 'A']->part->writeCluster(0, cluster);
	std::fill(cluster, cluster + 2048, 0);
	for (unsigned i = 1; i <= (n / (2048 * 8) + a); i++)
	{
		pw[part - 'A']->part->writeCluster(i, cluster);
	}
	pw[part - 'A']->rootDir = new RootDir(pw[part - 'A'], pw[part - 'A']->rootClustNum);
	PartWrap::FreeSegment s(rootCluster + 1, n - 2);
	if (pw[part - 'A']->isFormated == 0)
	{
		char* name = "Ova particija je formatirana";
		pw[part - 'A']->part->writeCluster(n - 1, name);
	}
	pw[part - 'A']->freeSegments.push_front(s);
	pw[part - 'A']->FUrequest = 0;
	pw[part - 'A']->isFormated = 1;
	lock.unlock();
	return 1;
}

char KernelFS::readRootDir(char part, EntryNum n, Directory & d)
{
	if (pw[part - 'A'] == nullptr) return 0;
	if (pw[part- 'A']->isFormated == 0) return 0;
	return pw[part - 'A']->rootDir->readRootDir(part, n, d);
}

char KernelFS::doesExist(char * fname)
{
	if (pw[fname[0] - 'A'] == nullptr) return 0;
	if (pw[fname[0] - 'A']->isFormated == 0) return 0;
	char *name = new char[10];
	char *ext = new char[5];
	splitChar(fname + 3, name, ext);
	if ((strlen(name) > 8) || (strlen(ext)>3)) return 0;
	if (pw[fname[0] - 'A']->rootDir->find(name, ext) == nullptr) return 0;
	else return 1;
}

File * KernelFS::open(char * fname, char mode)
{
	if (pw[fname[0] - 'A'] == nullptr) return 0;
	if (pw[fname[0] - 'A']->isFormated == 0) return nullptr;
	if (pw[fname[0] - 'A']->FUrequest == 1) return nullptr;
	char *name = new char[10];
	char *ext = new char[5];
	splitChar(fname + 3, name, ext);
	if ((strlen(name) > 8) || (strlen(ext)>3)) return nullptr;
	if (mode == 'r')
	{
		File *f = new File();
		f->myImpl = pw[fname[0] - 'A']->openRead(name, ext);
		return f;
	}
	else
	{
		File *f = new File();
		f->myImpl = pw[fname[0] - 'A']->openWrite(name, ext, mode);
		return f;
	}
}

char KernelFS::deleteFile(char * fname)
{
	if (pw[fname[0] - 'A'] == nullptr) return 0;
	if (pw[fname[0] - 'A']->isFormated == 0) return 0;
	if (pw[fname[0] - 'A']->FUrequest == 1) return 0;
	//unique_lock<mutex> lock(pw[fname[0] - 'A']->partMutex); Ovde treba da se 
	char *name = new char[10];
	char *ext = new char[5];
	splitChar(fname + 3, name, ext);
	if ((strlen(name) > 8) || (strlen(ext)>3)) return 0;
	return pw[fname[0] - 'A']->rootDir->deleteFile(name, ext);   
}

/* File * KernelFS::openNewFile(PartWrap::OpenFileListElem* op, ClusterNo size, ClusterNo num, char* fname, long fileNo, char mode)
{
	File* f = new File();
	KernelFile* kf = new KernelFile(size, num, fname, fileNo, mode);
	f->myImpl = kf;
	return f;
} */

void KernelFS::splitChar(const char *text, char *text1, char *text2)
{
	int len = (strchr(text, '.') - text)*sizeof(char);
	strncpy_s(text1, 10, text, len);
	strncpy_s(text2, 5, text + len + 1, 4);
}


