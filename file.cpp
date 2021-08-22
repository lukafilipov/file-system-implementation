#pragma once
#include "file.h"
#include "KernelFile.h"

File::File()
{

}
File::~File()
{
	delete myImpl;
}
char File::write(BytesCnt cnt , char *buffer)
{
	return myImpl->write(cnt, buffer);
}
BytesCnt File::read(BytesCnt cnt, char *buffer)
{
	return myImpl->read(cnt, buffer);
}
char File::seek(BytesCnt cnt)
{
	return myImpl->seek(cnt);
}
BytesCnt File::filePos()
{
	return myImpl->filePos();
}
char File::eof()
{
	return myImpl->eof();
}
BytesCnt File::getFileSize()
{
	return myImpl->getFileSize();
}
char File::truncate()
{
	return myImpl->truncate();
}
