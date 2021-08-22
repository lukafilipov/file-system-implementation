#pragma once

typedef unsigned long BytesCnt;
typedef unsigned long EntryNum;

const unsigned long ENTRYCNT = 64;
const unsigned int FNAMELEN = 8;
const unsigned int FEXTLEN = 3;

struct Entry 
{
	char name[FNAMELEN];
	char ext[FEXTLEN];
	char reserved;
	unsigned long indexCluster;
	unsigned long size;
};

typedef Entry Directory[ENTRYCNT];

class KernelFS;
class Partition;
class File;

class FS 
{
public:
	
	~FS();
	
	static char mount(Partition* partition);
	// montira particiju
	// vraca dodeljeno slovo

	static char unmount(char part);
	// demontira particiju oznacenu datim slovom
	// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha

	static char format(char part);
	// formatira particiju sa datim slovom;
	// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha

	static char readRootDir(char part, EntryNum n, Directory &d);
	// prvim argumentom se zadaje particija
	// drugim redni broj validnog ulaza od kog se pocinje citanje
	// treci argument je adresa na kojoj se smesta procitani niz ulaza

	static char doesExist(char* fname);
	// argument je naziv fajla sa apsolutnom putanjom

	static File* open(char* fname, char mode);
	static char deleteFile(char* fname);

protected:
	FS();
	static KernelFS *myImpl;
};

/* Open je staticka funkcija tako da fname mora da bude apsolutna putanja. U open u KernelFS 
   ce da se pravi i File f i KernelFile kf, sto moze jer je KernelFS prijateljska klasa klasi File.
   Dalje se da se uradi f->myImpl = kf, a konstruktoru kf ce da se proslede parametri open-a. */
