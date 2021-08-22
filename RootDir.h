#pragma once
#include "KernelFile.h"
#include "kernelFS.h"

class PartWrap;
class KernelFile;

class RootDir
{
	char *rootCluster, *curCluster;
	ClusterNo rootNo, curNum;
	PartWrap* myPW;
	ClusterNo numOfFiles, numOfLastFound;
	RootDir(PartWrap* p, ClusterNo n);
	~RootDir();
	KernelFile* find(char* name, char* ext, char mode = 'r');
	KernelFile* makeNewFile(char* name, char* ext);
	char readRootDir(char part, EntryNum n, Directory & d);
	char deleteFile(char* name, char *ext);
	void updateFileSize(ClusterNo fileNo, ClusterNo fileSize);
	friend class KernelFS;
	friend class PartWrap;
	friend class KernelFile;
};
