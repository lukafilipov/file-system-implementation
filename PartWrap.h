#pragma once
#include "Part.h"
#include <list>
#include <mutex>
#include "file.h"
#include "fs.h"
#include "RootDir.h"

class KernelFile;
class RootDir;
class Partition;

void ToChar(long from, char* buffer, int i);

long ToLong(char* buffer, int i);

using namespace std;

struct OpenFileListElem
{
	char *name, *ext, mode;
	int numOpen, numWaiting;
	ClusterNo size, indNum, fileNo;
	mutex *mut;
	condition_variable *con;
	OpenFileListElem(ClusterNo fsize, ClusterNo inNum, ClusterNo fileNum, char* names, char *exts, char mod)
	{
		size = fsize;
		indNum = inNum;
		fileNo = fileNum;
		name = names;
		ext = exts;
		mode = mod;
		numOpen = 0;
		mut = new mutex();
		con = new condition_variable();
		numWaiting = 0;

	}
	~OpenFileListElem()
	{
		delete mut;
		delete con;
	}
};

class PartWrap
{
	Partition* part;
	RootDir* rootDir;
	unsigned long nextCluster = 0;
	int numOpenFiles;
	bool FUrequest, isFormated;
	condition_variable partCon;
	mutex acquireWrite, partMutex;
	ClusterNo rootClustNum;
	struct FreeSegment
	{
		ClusterNo start, end;
		FreeSegment(ClusterNo a, ClusterNo b)
		{
			start = a;
			end = b;
		}
	};
	list<OpenFileListElem *> fileListHead;
	list<FreeSegment> freeSegments;
	PartWrap(Partition* p);
	~PartWrap();
	ClusterNo getFirstFree();
	void updateBitVector(ClusterNo numOfCluster, int value);
	bool checkFormated();
	KernelFile* openRead(char* name, char *ext);
	KernelFile* openWrite(char* fname, char* ext, char mode);
	OpenFileListElem* findOpenFile(char *name, char* ext);
	friend class KernelFile;
	friend class KernelFS;
	friend class RootDir;
};
