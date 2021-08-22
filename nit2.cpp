#include"testprimer.h"

DWORD WINAPI nit2run(){
	//1.blok
	wait(mutex); partition2=new Partition("p2.ini"); signal(mutex);
	wait(mutex); cout<<"Nit2: Kreirana particija"<<endl; signal(mutex);
	p2=FS::mount(partition2);
	wait(mutex); cout<<"Nit2: Montirana particija"<<endl; signal(mutex);
	FS::format(p2);
	wait(mutex); cout<<"Nit2: Formatirana particija"<<endl; signal(mutex);
	{
		char filepath[]="2:\\fajl2.dat";
		filepath[0]=p2;
		File *f=FS::open(filepath,'w');
		wait(mutex); cout<<"Nit2: Kreiran fajl 'fajl2.dat'"<<endl; signal(mutex);
		f->write(ulazSize,ulazBuffer);
		wait(mutex); cout<<"Nit2: Prepisan sadrzaj 'ulaz.dat' u 'fajl2.dat'"<<endl; signal(mutex);
		delete f;
		wait(mutex); cout<<"Nit2: zatvoren fajl 'fajl2.dat'"<<endl; signal(mutex);
	}	
	wait(mutex); cout<<"Nit2: wait 1"<<endl; signal(mutex);
	wait(sem12);//ceka nit 1
	wait(mutex); cout<<"Nit2: signal 3"<<endl; signal(mutex);
	signal(sem23);//signalizira nit 3 - 5.signal
	wait(mutex); cout<<"Nit2: wait 3"<<endl; signal(mutex);
	wait(sem32);//ceka nit 3

	//2.blok
	{
		File *src,*dst;
		char filepath1[]="1:\\fajl1.dat";
		filepath1[0]=p1;
		src=FS::open(filepath1,'r');
		src->seek(src->getFileSize()/2);//pozicionira se na pola fajla
		wait(mutex); cout<<"Nit2: Otvoren fajl 'fajl1.dat' i pozicionirani smo na polovini"<<endl; signal(mutex);
		char filepath2[]="1:\\fajl5.dat";
		filepath2[0]=p1;
		dst=FS::open(filepath2,'w');
		wait(mutex); cout<<"Nit2: Otvoren fajl 'fajl5.dat'"<<endl; signal(mutex);
		char c;
		while(!src->eof()){
			src->read(1,&c);
			dst->write(1,&c);
		}
		wait(mutex); cout<<"Nit2: Prepisana druga polovina 'fajl1.dat' u 'fajl5.dat'"<<endl; signal(mutex);
		delete dst;
		wait(mutex); cout<<"Nit2: Zatvoren fajl 'fajl5.dat'"<<endl; signal(mutex);
		delete src;
		wait(mutex); cout<<"Nit2: Zatvoren fajl 'fajl1.dat'"<<endl; signal(mutex);
	}
	wait(mutex); cout<<"Nit2: wait 1"<<endl; signal(mutex);
	wait(sem12);//ceka nit1

	//3.blok
	{
		File *f2,*f4,*f5;
		char filepath1[]="2:\\fajl2.dat";
		filepath1[0]=p2;
		f2=FS::open(filepath1,'w');
		wait(mutex); cout<<"Nit2: Otvoren fajl 'fajl2.dat'"<<endl; signal(mutex);
		char filepath2[]="2:\\fajl4.dat";
		filepath2[0]=p2;
		f4=FS::open(filepath2,'r');
		wait(mutex); cout<<"Nit2: Otvoren fajl 'fajl4.dat'"<<endl; signal(mutex);
		char c;
		while(!f4->eof()){
			f4->read(1,&c);
			f2->write(1,&c);
		}
		wait(mutex); cout<<"Nit2: 'fajl4.dat' iskopiran u 'fajl2.dat'"<<endl; signal(mutex);
		delete f4;
		wait(mutex); cout<<"Nit2: Zatvoren fajl 'fajl4.dat'"<<endl; signal(mutex);
		delete f2;
		wait(mutex); cout<<"Nit2: Zatvoren fajl 'fajl2.dat'"<<endl; signal(mutex);
		f2=FS::open(filepath1,'a');
		wait(mutex); cout<<"Nit2: Otvoren fajl 'fajl2.dat'"<<endl; signal(mutex);
		char filepath3[]="1:\\fajl5.dat";
		filepath3[0]=p1;
		f5=FS::open(filepath3,'r');
		wait(mutex); cout<<"Nit2: Otvoren fajl 'fajl5.dat'"<<endl; signal(mutex);
		while(!f5->eof()){
			f5->read(1,&c);
			f2->write(1,&c);
		}
		wait(mutex); cout<<"Nit2: 'fajl5.dat' dopisan na kraj 'fajl2.dat'"<<endl; signal(mutex);
		delete f5;
		wait(mutex); cout<<"Nit2: Zatvoren fajl 'fajl5.dat'"<<endl; signal(mutex);
		delete f2;
		wait(mutex); cout<<"Nit2: Zatvoren fajl 'fajl2.dat'"<<endl; signal(mutex);
	}
	signal(sem23);//11. signal

	//4.blok
	{
		Entry D[64];		
		int lim = FS::readRootDir(p1, 0, D);
		wait(mutex);
		cout << "Nit2: " << p1 << " root dir:" << endl;
		cout << '\t' << p1 << ':' << endl;
		for (int i = 0; i<lim; i++) cout << "\t" << ((i != lim - 1) ? char(195) : char(192)) << ' ' << D[i] << ' ' << D[i].ext << ' ' <<D[i].indexCluster << endl;
		signal(mutex);
		char filepath[]="1:\\fajl5.dat";
		filepath[0]=p1;
		FS::deleteFile(filepath);
		lim = FS::readRootDir(p2, 0, D);
		wait(mutex);
		cout << "Nit2: " << p2 << " root dir:" << endl;
		cout << '\t' << p2 << ':' << endl;
		for (int i = 0; i<lim; i++) cout << "\t" << ((i != lim - 1) ? char(195) : char(192)) << ' ' << D[i] << endl;
		signal(mutex);
		wait(mutex); cout<<"Nit2: Obrisan fajl 'fajl5.dat'"<<endl; signal(mutex);
		FS::unmount(p1);
		wait(mutex); cout<<"Nit2: Demontirana p1 particija"<<endl; signal(mutex);
	}
	wait(mutex); cout<<"Nit2: signal 1"<<endl; signal(mutex);
	signal(sem21);//12. signal

	wait(mutex); cout<<"Nit2: Zavrsena!"<<endl; signal(mutex);
	signal(semMain);
	return 0;
}