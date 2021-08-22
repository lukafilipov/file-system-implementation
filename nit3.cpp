#include"testprimer.h"

DWORD WINAPI nit3run(){
	wait(sem23);	//ceka nit2
	{
		char filepath1[]="1:\\fajl1.dat";
		filepath1[0]=p1;
		File *f1=FS::open(filepath1,'r');
		wait(mutex); cout<<"Nit3: Otvoren fajl fajl1.dat"<<endl;
		cout<<"Nit3: signal 2"<<endl; signal(mutex);
		signal(sem32);//6.signal
		char filepath2[]="1:\\fajl3.dat";
		filepath2[0]=p1;
		File *f2=FS::open(filepath2,'w');
		wait(mutex); cout<<"Nit3: Kreiran fajl fajl3.dat"<<endl; signal(mutex);
		wait(mutex); cout<<"Nit3: signal 1"<<endl; signal(mutex);
		signal(sem31);//7.signal
		char c;
		while(!f1->eof()){
			f1->read(1,&c);
			f2->write(1,&c);
		}
		delete f1;
		wait(mutex); cout<<"Nit3: Zatvoren fajl fajl1.dat"<<endl; signal(mutex);
		delete f2;
		wait(mutex); cout<<"Nit3: Zatvoren fajl fajl3.dat"<<endl; signal(mutex);
	}
	wait(mutex); cout<<"Nit3: wait 1"<<endl; signal(mutex);
	wait(sem13);//ceka nit 1

	//4.blok
	{
		char filepath[]="2:\\fajl2.dat";
		filepath[0]=p2;
		File *f=FS::open(filepath,'r');
		wait(mutex); cout<<"Nit3: Otvoren fajl fajl2.dat"<<endl; signal(mutex);
		delete f;
		wait(mutex); cout<<"Nit3: Zatvoren fajl fajl2.dat"<<endl; signal(mutex);
	}
	wait(mutex); cout<<"Nit3: wait 1"<<endl; signal(mutex);
	wait(sem13);
	wait(mutex); cout<<"Nit3: wait 2"<<endl; signal(mutex);
	wait(sem23);

	//5.blok
	{
		char filepath[]="2:\\fajl2.dat";
		filepath[0]=p2;
		File *f=FS::open(filepath,'r');
		wait(mutex); cout<<"Nit3: Otvoren fajl fajl2.dat"<<endl; signal(mutex);
		ofstream fout("izlaz1.dat", ios::out|ios::binary);
		char *buff=new char[f->getFileSize()];
		f->read(f->getFileSize(),buff);
		fout.write(buff,f->getFileSize());
		wait(mutex); cout<<"Nit3: Upisan 'fajl2.dat' u fajl os domacina 'izlaz1.dat'"<<endl; signal(mutex);
		delete [] buff;
		fout.close();
		delete f;
		wait(mutex); cout<<"Nit3: Zatvoren fajl fajl2.dat"<<endl; signal(mutex);
		Entry D[64];
		int lim = FS::readRootDir(p2, 0, D);
		wait(mutex);
		cout << "Nit3: " << p2 << " root dir:" << endl;
		cout << '\t' << p2 << ':' << endl;
		for (int i = 0; i<lim; i++) cout << "\t" << ((i != lim - 1) ? char(195) : char(192)) << ' ' << D[i] << endl;
		signal(mutex);
		FS::unmount(p2);
		wait(mutex); cout<<"Nit3: Demontirana particija p2"<<endl; signal(mutex);
	}

	wait(mutex); cout<<"Nit3: Zavrsena!"<<endl; signal(mutex);
	signal(semMain);
	return 0;
}