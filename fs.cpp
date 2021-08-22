#pragma once
#include "fs.h"
#include "KernelFS.h"
#include <iostream>

FS::~FS()
{
}

char FS::mount(Partition * partition)
{
	return myImpl->mount(partition);
}

char FS::unmount(char part)
{
	return myImpl->unmount(part);
}

char FS::format(char part)
{
	return myImpl->format(part);
}

char FS::readRootDir(char part, EntryNum n, Directory & d)
{
	return myImpl->readRootDir(part, n, d);
}

char FS::doesExist(char * fname)
{
	return myImpl->doesExist(fname);
}

File * FS::open(char * fname, char mode)
{
	if ((mode != 'r') && (mode != 'w') && (mode != 'a'))
	{
		cout << "Nevalidno otvaranje fajla" << endl;
		return nullptr;
	}
	else return myImpl->open(fname, mode);
}

char FS::deleteFile(char * fname)
{
	return myImpl->deleteFile(fname);
}

FS::FS()
{
}
