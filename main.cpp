#include"testprimer.h"

using namespace std;

HANDLE nit1,nit2,nit3;
DWORD ThreadID;

HANDLE semMain=CreateSemaphore(NULL,0,32,NULL);
HANDLE sem12=CreateSemaphore(NULL,0,32,NULL);
HANDLE sem13=CreateSemaphore(NULL,0,32,NULL);
HANDLE sem21=CreateSemaphore(NULL,0,32,NULL);
HANDLE sem23=CreateSemaphore(NULL,0,32,NULL);
HANDLE sem31=CreateSemaphore(NULL,0,32,NULL);
HANDLE sem32=CreateSemaphore(NULL,0,32,NULL);
HANDLE mutex=CreateSemaphore(NULL,1,32,NULL);

Partition *partition1, *partition2;
char p1,p2;

char *ulazBuffer;
int ulazSize;

ostream& operator<<(ostream &os,const Entry &E){
	char name[13];
	memcpy(name,E.name,8);
	name[8]='.';
	memcpy(name+9,E.ext,3);
	name[12]=0;
	return os<<name<<" ["<<E.size<<']';
}

int main(){
	clock_t startTime,endTime;
	cout<<"Pocetak testa!"<<endl;
	startTime=clock();//pocni merenje vremena

	{//ucitavamo ulazni fajl u bafer, da bi nit 1 i 2 mogle paralelno da citaju
		FILE *f=fopen("ulaz.dat","rb");
		if(f==0){
			cout<<"GRESKA: Nije nadjen ulazni fajl 'ulaz.dat' u os domacinu!"<<endl;
			system("PAUSE");
			return 0;//exit program
		}
		ulazBuffer=new char[32*1024*1024];//32MB
		ulazSize=fread(ulazBuffer, 1, 32*1024*1024, f);
		fclose(f);
	}

	nit1=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) nit1run,NULL,0,&ThreadID); //kreira i startuje niti
	nit2=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) nit2run,NULL,0,&ThreadID);
	nit3=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) nit3run,NULL,0,&ThreadID);

	for(int i=0; i<3; i++) wait(semMain);//cekamo da se niti zavrse
	delete [] ulazBuffer;
	endTime=clock();
	cout<<"Kraj test primera!"<<endl;
	cout<<"Vreme izvrsavanja: "<<((double)(endTime-startTime)/((double)CLOCKS_PER_SEC/1000.0))<<"ms!"<<endl;
	CloseHandle(mutex);
	CloseHandle(semMain);
	CloseHandle(sem12);
	CloseHandle(sem13);
	CloseHandle(sem21);
	CloseHandle(sem23);
	CloseHandle(sem31);
	CloseHandle(sem32);
	CloseHandle(nit1);
	CloseHandle(nit2);
	CloseHandle(nit3);
	return 0;
}