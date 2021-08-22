#pragma once
#include "Part.h"
#include "PartWrap.h"
#include "string"
#include <list>
#include <mutex>
#include "KernelFile.h"
#include "kernelFS.h"
#include "PartWrap.h"

using namespace std;

class PartWrap;

class KernelFS
{
	static PartWrap *pw[26];
	

	KernelFS();
	static char mount(Partition* partition);
	static char unmount(char part);
	static char format(char part);
	static char readRootDir(char part, EntryNum n, Directory &d);
	static char doesExist(char* fname);
	static File* open(char* fname, char mode);
	static char deleteFile(char* fname);
	static void splitChar(const char *text, char *text1, char *text2);
	friend class KernelFile;
	friend class PartWrap;
	friend class RootDir;
	friend class FS;

};

