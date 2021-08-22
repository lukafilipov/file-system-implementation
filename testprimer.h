#pragma once

#include"fs.h"
#include"file.h"
#include"part.h"

#include<iostream>
#include<fstream>
#include<cstdlib>
#include<windows.h>
#include<ctime>

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

using namespace std;

extern HANDLE nit1,nit2,nit3;
extern DWORD ThreadID;

extern HANDLE mutex;
extern HANDLE semMain;
extern HANDLE sem12;
extern HANDLE sem13;
extern HANDLE sem21;
extern HANDLE sem23;
extern HANDLE sem31;
extern HANDLE sem32;

extern Partition *partition1, *partition2;
extern char p1,p2;

extern char *ulazBuffer;
extern int ulazSize;

ostream& operator<<(ostream &os,const Entry &E);
DWORD WINAPI nit1run();
DWORD WINAPI nit2run();
DWORD WINAPI nit3run();