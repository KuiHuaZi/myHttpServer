/*
 * process_poll.h
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#ifndef SRC_PROCESS_POOL_H_
#define SRC_PROCESS_POOL_H_
#include<unistd.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<signal.h>
#include<sys/timerfd.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>
#include"process_pool.h"
#include"time_heap.h"
#include"connect_pool.h"
#include "common_functions.h"
template<class T>
class ProcessPool;
class Process
{
public:
	template<class T> friend class ProcessPool;
	Process():_pid(-1),_connect_number(0){}
private:
	pid_t _pid;
	int _pipefd[2];//use for connect with main process
	int _connect_number;
};
template< class T>
class ProcessPool
{
public:
	static ProcessPool<T>& instance(int listenfd,int process_number)
	{
		static ProcessPool<T>processpool(listenfd,process_number);
		return processpool;
	}
	~ProcessPool()
	{
		delete[]_sub_process;
	}
	void Run();
private:
	ProcessPool(int listenfd,int process_number = 8);
	ProcessPool(ProcessPool<T> const&);
	ProcessPool<T>& operator=(ProcessPool<T> const&);
	void RunChild();
	void RunParent();
	void SetupSigPipe();
	int GetLeastConnectNumberProcess();

private:
	int _listenfd;
	int _epollfd;
	int _process_number;
	bool _stop;
	Process *_sub_process;
	int _index;
	int _connect_number;
	static const int MAX_PROCESS_NUMBER = 16;
	static const int USER_PER_PROCESS = 65536;
	static const int MAX_EVENT_NUMBER = 10000;
	static const int CONNECT_KEEP_TIME = 100;


};
static int sig_pipefd[2];
void SigHandler(int sig)
{
	int save_errno = errno;
	int msg = sig;
	send(sig_pipefd[1],(char*)&msg,1,0);
	errno = save_errno;
}
void AddSig(int sig,void(*handler)(int),bool restart = true)
{
	struct sigaction sa;
	bzero(&sa,sizeof(sa));
	sa.sa_handler= handler;
	if(restart)
	{
		sa.sa_flags|=SA_RESTART;
	}

	sigfillset(&sa.sa_mask);
	assert(sigaction(sig,&sa,NULL)!=-1);

}
template<class T>
ProcessPool<T>::ProcessPool(int listenfd,int process_number)
:_listenfd(listenfd),_process_number(process_number),_stop(false),_index(-1),_epollfd(-1),_connect_number(-1)
{

	assert(listenfd>=0);
	assert((process_number<MAX_PROCESS_NUMBER)&&(process_number>0));
	_sub_process = new Process[_process_number];

	assert(_sub_process);
	for(int i = 0 ;i < _process_number;i++)
	{
		int ret = socketpair(AF_UNIX,SOCK_STREAM,0,_sub_process[i]._pipefd);
		assert(ret==0);
		_sub_process[i]._pid = fork();
		assert(_sub_process[i]._pid!=-1);
		if(_sub_process[i]._pid == 0)
		{
			close(_sub_process[i]._pipefd[0]);//in child process
			_index = i;
			_connect_number = 0;
			break;
		}
		else
		{
			close(_sub_process[i]._pipefd[1]);//in parent process
			continue;
		}
	}

}
template<class T>
void ProcessPool<T>::SetupSigPipe()
{
	_epollfd = epoll_create(5);
	assert(_epollfd!=-1);
	int ret = socketpair(AF_UNIX,SOCK_STREAM,0,sig_pipefd);
	assert(ret!=-1);

	SetNonblocking(sig_pipefd[1]);
	AddFd(_epollfd,sig_pipefd[0]);

	AddSig(SIGCHLD,SigHandler);
	AddSig(SIGTERM,SigHandler);
	AddSig(SIGINT,SigHandler);
//	AddSig(SIGALRM,SigHandler);
	AddSig(SIGPIPE,SIG_IGN);
}
template<class T>
void ProcessPool<T>::Run()
{
	if(_index==-1)
	{
		RunParent();
		return;
	}
	RunChild();
}
template<class T>
int ProcessPool<T>::GetLeastConnectNumberProcess()
{
	int min = _sub_process[0]._connect_number;
	int index = 0;
	for(int i = 1;i<_process_number;++i)
	{
		if(min>_sub_process[i]._connect_number)
		{
			min = _sub_process[i]._connect_number;
			index = i;
		}
	}
	return index;
}
template<class T>
void ProcessPool<T>::RunParent()
{
	SetupSigPipe();
	assert(_epollfd!=-1);
	AddFd(_epollfd,_listenfd);
	uint32_t ev = EPOLLIN|EPOLLET;
	ModifyFd(_epollfd,_listenfd,ev);
	int nfds = 0 ;
	int number_child_process = _process_number;
	int count_of_connect = 0;
	int new_connect = 1;
	struct epoll_event evlist[10] ;
	for(int j = 0;j<_process_number;++j)
	{
		AddFd(_epollfd,_sub_process[j]._pipefd[0]);
	}
	while(!_stop)
	{

		 nfds = epoll_wait(_epollfd,evlist,2,-1);
		 if((nfds < 0) && (errno != EINTR))
		 {
			 printf("RunParent:epoll_wait failed!\n");
			 break;
		 }
		 for(int i = 0; i < nfds; ++i)
		 {
			 int sockfd = evlist[i].data.fd;
			 if((sockfd == sig_pipefd[0]) && (evlist[i].events & EPOLLIN))
			 {
				int sig;
				char signals[1024];
				int recv_count = recv(sockfd,signals,sizeof(signals),0);
				if(recv_count<0)
				{
					continue;
				}
				else
				{
					for(int j = 0; j < recv_count; ++j)
					{
						switch(signals[j])
						{
						case SIGCHLD:
							printf("Signal:SIGCHLD\n");
							pid_t child_pid;
							int stat;
							while((child_pid = waitpid(-1,&stat,WNOHANG))>0)
							{
								--number_child_process;
								if(number_child_process == 0)
								{
									_stop = true;
								}
								for(int k = 0; k < _process_number; ++k)
								{
									if(_sub_process[k]._pid == child_pid)
									{
										_sub_process[k]._pid = -1;
										close(_sub_process[k]._pipefd[0]);
										printf("child process %d end!\n",child_pid);
									}
								}

							}
						break;
						case SIGTERM:
						case SIGINT:
						{
							printf("kill all child !\n");
							for(int k = 0; k < _process_number; ++k)
							{
								if(_sub_process[k]._pid!=-1)
								{
									int sig = -1;
									send(_sub_process[k]._pipefd[0],(char*)&sig,sizeof(sig),0);

								}
							}
							break;
						}
						default:
							break;
						}
					}
				}
			 }
			 else if(sockfd == _listenfd)
			 {
				/* if(count_of_connect > _process_number)
				 {
					 count_of_connect %= _process_number;
				 }
				 if(_sub_process[count_of_connect] == -1)
				 {
					 _stop = true;
					 break;
				 }
				 send(_sub_process[count_of_connect]._pipefd[0],(char*)&new_connect,sizeof(new_connect),0);
				 printf("send request to child %d\n",count_of_connect);
				 ++count_of_connect;*/
	                int index = GetLeastConnectNumberProcess();
	                send( _sub_process[index]._pipefd[0], ( char* )&new_connect, sizeof( new_connect ), 0 );
					printf("send request to child %d\n",_sub_process[index]._pid);
			 }
			 else if(evlist[i].events & EPOLLIN)
			 {
	                int connect_number = 0;
	                int ret = recv( sockfd, ( char* )&connect_number, sizeof( connect_number ), 0 );
	                if( ( ( ret < 0 ) && ( errno != EAGAIN ) ) || ret == 0 )
	                {
	                    continue;
	                }
	                for( int i = 0; i < _process_number; ++i )
	                {
	                    if( sockfd == _sub_process[i]._pipefd[0] )
	                    {
	                        _sub_process[i]._connect_number = connect_number;
	            			printf("sub Process:%d have connect number:%d\n",_sub_process[i]._pid,connect_number);
	                        break;
	                    }
	                }
			 }

		 }

	}
}
template<class T>
void ProcessPool<T>::RunChild()
{
	SetupSigPipe();
	int my_pid=_sub_process[_index]._pid = getpid();
	int pipefd_with_parent =_sub_process[_index]._pipefd[1];
	AddFd(_epollfd,pipefd_with_parent);
	TimerHeap timers(USER_PER_PROCESS);
	struct epoll_event evlist[MAX_EVENT_NUMBER];
	int time_fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
	AddFd(_epollfd,time_fd);
	uint32_t ev = EPOLLIN|EPOLLET;
	ModifyFd(_epollfd,time_fd,ev);
/*	while(!AddFd(_epollfd,time_fd))
	{
		printf("Process %d RunChild:add time_fd failed!\n",_sub_process[_index]._pid);
	}*/
	ConnectPool<T> connect_pool(USER_PER_PROCESS);
	int ret = -1;
	while(!_stop)
	{
		int nfds = epoll_wait(_epollfd,evlist,MAX_EVENT_NUMBER,-1);
		if((nfds<0) && (errno!=EINTR))
		{
			printf("Process %d RunChild:epoll failed!\n",my_pid);
			continue;
		}
		int *expire_fd = nullptr;
		int old_connect_number = _connect_number;
		for(int i = 0; i < nfds; ++i)
		{
			int sockfd = evlist[i].data.fd;
			if((sockfd == pipefd_with_parent)&&(evlist[i].events & EPOLLIN))
			{
				int tmp = 0;
				ret = recv(sockfd,(char*)&tmp,sizeof(tmp),0);
				if(((ret<0)&&(errno!=EAGAIN||errno!=EINTR))||ret == 0)
				{
					continue;
				}
				else if(tmp ==1)
				{
					if(_connect_number >=USER_PER_PROCESS)
					{
						continue;
					}
					int newfd = accept(_listenfd,NULL,0);
					if(newfd<0)
					{
						printf("Process %d RunChilde:accept failed!\n",my_pid);
						continue;
					}
					if(!AddFd(_epollfd,newfd))
					{
						printf("Process %d RunChilde:AddFd failed!\n",my_pid);
						close(newfd);
						continue;
					}
					if(!connect_pool.AddConnect(newfd,CONNECT_KEEP_TIME))
					{
						RemoveFd(_epollfd,newfd);
						continue;
					}
					Timer &timer = connect_pool.TimerOfConnect(newfd);
					timers.InsertTimer(timer);
					printf("Add new fd:%d\n",newfd);
					if(timers.size()==1)
					{
						struct itimerspec ts;
						memset(&ts,0,sizeof(ts));
						ts.it_value.tv_sec = timers.Min().Expire();
						int flag = TIMER_ABSTIME;
						ret = timerfd_settime(time_fd,flag,&ts,NULL);
						assert(ret == 0);
					}
					_connect_number++;

				}
				else if(tmp == -1)
				{
					_stop = true;
				}
			}
			else if((sockfd == sig_pipefd[0])&&(evlist[i].events & EPOLLIN))
			{
				int sig;
				char signals[1024];
				ret = recv(sig_pipefd[0],signals,sizeof(signals),0);
				if(ret<0)
				{
					continue;
				}
				for(int j = 0; j < ret; ++j)
				{
					switch(signals[i])
					{
					case SIGCHLD:
						pid_t pid;
						int stat;
						while((pid = waitpid(-1,&stat,WNOHANG))>0)
						{
							continue;
						}
						break;
					case SIGTERM:
					case SIGINT:
						_stop = true;
						break;
					default:
						break;
					}
				}
			}
			else if(sockfd == time_fd)
			{
				expire_fd = timers.GetExpireAndSetNewTimer();
				printf("Timer is expired!\n");
			}
			else if(evlist[i].events & EPOLLIN)
			{
				ReturnCode code = connect_pool.Process(sockfd,READ);
				uint32_t ev = EPOLLOUT;
				printf("update timer!\n");
				Timer &timer = connect_pool.TimerOfConnect(sockfd);
				timer.AdjustTimer(CONNECT_KEEP_TIME);
				timers.UpdateTimer(timer);
				switch(code)
				{
				case TOWRITE:
					ModifyFd(_epollfd,sockfd,ev);
					printf("read to write!\n");
					break;
				case TOCLOSE:
					timers.DelTimer(timer);
					RemoveFd(_epollfd,sockfd);
					connect_pool.RecyleConn(sockfd);
					_connect_number--;
					break;
				case TOREAD:
				case CONTINUE:
				default:
					break;
				}
			}
			else if(evlist[i].events & EPOLLOUT)
			{
				printf("connection:%d have data to write!\n",sockfd);
				ReturnCode code = connect_pool.Process(sockfd,WRITE);
				uint32_t ev= EPOLLIN;
				switch(code)
				{
				case TOREAD:
					ModifyFd(_epollfd,sockfd,ev);
					printf("WRITE to READ\n");
					break;
				case TOCLOSE:
					printf("WRITE to CLOSE\n");
					timers.DelTimer(connect_pool.TimerOfConnect(sockfd));
					connect_pool.RecyleConn(sockfd);
					RemoveFd(_epollfd,sockfd);
					_connect_number--;
					break;
				case TOWRITE:
				case CONTINUE:
				default:
					printf("CONTINUE!\n");
				}
			}
			else
			{
				continue;
			}

		}
		if(expire_fd)
		{
			for(int i = 0; expire_fd[i]!=END;++i)
			{
				connect_pool.RecyleConn(expire_fd[i]);
				RemoveFd(_epollfd,expire_fd[i]);
				_connect_number--;
				printf("Remove fd:%d\n",expire_fd[i]);
			}
			struct itimerspec ts;
			memset(&ts,0,sizeof(ts));
			if(!timers.IsEmpty())
			{
				ts.it_value.tv_sec = timers.Min().Expire();
				int flag = TIMER_ABSTIME;
				int ret = timerfd_settime(time_fd,flag,&ts,NULL);
				assert(ret == 0);
			}
		}
		if(old_connect_number!=_connect_number)
		{
			printf("Process:%d have connect number:%d\n",my_pid,_connect_number);
			send(pipefd_with_parent,&_connect_number,sizeof(_connect_number),0);
		}

	}
close(_epollfd);
close(pipefd_with_parent);

}
#endif /* SRC_PROCESS_POOL_H_ */
