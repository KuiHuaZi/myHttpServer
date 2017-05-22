//============================================================================
// Name        : HTTPserver.cpp
// Author      : Amapola
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<iostream>
#include<sys/sem.h>
#include<errno.h>
#include<sys/wait.h>
using namespace std;
union semun
{
	int val;
	struct semid_ds* buf;
	unsigned short int *array;
	struct seminfo* _buf;
};
int main(void) {

	cout<<"This is Parent process with pid: "<<(int)getpid()<<endl;
	int sem_id;
	if((sem_id = semget(IPC_PRIVATE,1,0666))==-1)
		cerr<<"segmget faliled ,erron :"<<errno<<endl;
	union semun sem_un;
	sem_un.val =1;
	semctl(sem_id,0,SETVAL,sem_un);
	pid_t child_pid = fork();
	if(child_pid>0)
	{
		struct sembuf s = {0,-1,SEM_UNDO};
		if(semop(sem_id,&s,1)==0)
		{
		cout<<"parent:the semop success!"<<endl;
		sleep(5);
		cout<<"parent:fork success,the childe pid is "<<(int)child_pid<<endl;
		s.sem_op = 1;
		semop(sem_id,&s,1);
		}
		else
			cerr<<"parent:semop failed!"<<endl;
		int stat_loc;
		if((int)wait(&stat_loc)>0)
		{

			if(semctl(sem_id,0,IPC_RMID,sem_un)!=0)
			{
				cerr<<"parent:semctl error!"<<endl;
			}
		}
	}
	else if(child_pid ==0)
	{
		struct sembuf s = {0,-1,SEM_UNDO};
		if(semop(sem_id,&s,1)==0)
		{
		cout<<"child:the semop success!"<<endl;
		sleep(5);
		cout<<"child:fork success,the childe pid is "<<(int)getpid()
				<<"the parent pid is "<<(int)getppid()<<endl;
		s.sem_op = 1;
		semop(sem_id,&s,1);
		}
		else
			cerr<<"childe:semop failed!"<<endl;
	}
	else
		cerr<<"fork failed"<<endl;

	return 1;
}
