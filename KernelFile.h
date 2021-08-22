#pragma once
#include "file.h"
#include "fs.h"
#include "Part.h"
#include "KernelFS.h"
#include "PartWrap.h"
#include <string>

using namespace std;

class PartWrap;
struct OpenFileListElem;

class KernelFile
{
	long const MAXSIZE = 256 * 2048 + 256 * 512 * 2048;
	char mode, part;
	char* name, *ext;
	long fileNo, cachePos= -1;
	bool in2d = 0, dirty = 0;
	ClusterNo size, oldpos, pos, in2num, indNum, cacheNum = 0;
	unsigned lastindex1, lastindex2;
	PartWrap* myPW;
	char *index1, *index2, *cacheClust = nullptr;
	OpenFileListElem* myEntry = nullptr;

	KernelFile(PartWrap *myPW, ClusterNo size, ClusterNo indNum, char* name, char* ext, long fileNo, char mode = 'r');
	~KernelFile();
	char write(BytesCnt, char *buffer);
	BytesCnt read(BytesCnt, char *buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
	char allocateCluster();
	ClusterNo findCluster();
	void updateCache();
	friend class File;
	friend class KernelFS;
	friend class RootDir;
	friend class PartWrap;
};

