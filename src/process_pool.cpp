/*
 * process_pool.cpp
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#include<unistd.h>
#include<pthread.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<signal.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/types.h>
#include"process_pool.h"
#include"time_heap.h"
#include"connect_pool.h"
#include "common_functions.h"
static int sig_pipefd[2];
void SigHandler(int sig)
{
	int save_errno = errno;
	int msg = sig;
	send(sig_pipefd[1],(char*)&msg,1,0);
	errno = save_errno;


}
void AddSig(int sig,void(handler)(int),bool restart = true)
{
	struct sigaction sa;
	bzero(&sa,sizeof(sa));
	sa.__sigaction_handler = handler;
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
		int ret = socketpair(AF_UNIX,SOCK_STREAM,0,_sub_process[i].m_pipefd);
		assert(ret==0);
		_sub_process[i].m_pid = fork();
		assert(_sub_process[i].m_pid!=-1);
		if(_sub_process[i].m_pid == 0)
		{
			close(_sub_process[i].m_pipefd[0]);//in child process
			_index = i;
			_connect_number = 0;
			break;
		}
		else
		{
			close(_sub_process[i].m_pipefd[1]);//in parent process
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
	AddSig(SIGALRM,SigHandler);
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
void ProcessPool<T>::RunParent()
{
	SetupSigPipe();
	AddFd(_epollfd,_listenfd);
	int nfds = 0 ;
	int number_child_process = _process_number;
	int count_of_connect = 0;
	int new_connect = 1;
	struct epoll_event evlist[2] ;
	while(_stop)
	{

		 nfds = epoll_wait(_epollfd,evlist,2,-1);
		 if((nfds < 0) && (errno != EINTR))
		 {
			 printf("RunParent:epoll_wait failed!\n");
			 break;
		 }
		 int i;
		 for(i = 0; i < nfds; ++i);
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
						switch(signals[i])
						{
						case SIGCHLD:
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
									if(_sub_process[k].m_pid == child_pid)
									{
										_sub_process[k].m_pid = -1;
										close(_sub_process[k].m_pipefd[0]);
										printf("child process %d end!\n",k);
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
								if(_sub_process[k].m_pid!=-1)
								{
									int sig = -1;
									send(_sub_process[k].m_pipefd[0],(char*)&sig,sizeof(sig),0);

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
				 if(count_of_connect > _process_number)
				 {
					 count_of_connect %= _process_number;
				 }
				 if(_sub_process[count_of_connect] == -1)
				 {
					 _stop = true;
					 break;
				 }
				 send(_sub_process[count_of_connect].m_pipefd[0],(char*)&new_connect,sizeof(new_connect),0);
				 printf("send request to child %d\n",count_of_connect);
				 ++count_of_connect;
			 }
			 else
				 continue;
		 }

	}
}
template<class T>
void ProcessPool<T>::RunChild()
{
	SetupSigPipe();
	int pipefd_with_parent =_sub_process[_index].m_pipefd[1];
	AddFd(_epollfd,pipefd_with_parent);
	TimerHeap heap(CAP_OF_TIMERHEAP);
	struct epoll_event evlist[MAX_EVENT_NUMBER];
	int time_fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
	while(!AddFd(_epollfd,time_fd))
	{
		printf("Process %d RunChild:add time_fd failed!\n",_sub_process[_index].m_pid);
	}
	ConnectPool<T> connections(CAP_OF_TIMERHEAP,_epollfd);
	int ret = -1;
	while(!_stop)
	{
		int nfds = epoll_wait(_epollfd,evlist,MAX_EVENT_NUMBER,-1);
		if((nfds<0) && (errno!=EINTR))
		{
			printf("Process %d RunChild:epoll failed!\n",_sub_process[_index].m_pid);
			break;
		}
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
					int newfd = accept(_listenfd,NULL,0);
					if(newfd<0)
					{
						printf("Process %d RunChilde:accept failed!\n",_sub_process[_index].m_pid);
						continue;
					}
					if(!AddFd(_epollfd,newfd))
					{
						printf("Process %d RunChilde:AddFd failed!\n",_sub_process[_index].m_pid);
						continue;
					}
					if(!connections.AddConnect(newfd))
					{
						RemoveFd(_epollfd,newfd);
						continue;
					}
					heap.InsertTimer(connections.TimerOfConnect(newfd));
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
				//todo:trick
				int *expire_fd = heap.GetExpireAndSetNewTimer();
				for(int i = 0; expire_fd[i]!=END;++i)
				{
					connections.RecyleConn(expire_fd[i]);
					RemoveFd(_epollfd,expire_fd[i]);
				}
				delete[]expire_fd;

			}
			else if(evlist[i].events & EPOLLIN)
			{
				heap.UpdateTimer(connections.TimerOfConnect(sockfd));
				ReturnCode ret = connections.Process(sockfd,READ);
				switch(ret)
				{
				case TOWRITE:
					uint32_t ev = EPOLLOUT;
					ModifyFd(_epollfd,sockfd,ev);
					break;
				case TOCLOSE:
					RemoveFd(_epollfd,sockfd);
					connections.RecyleConn(sockfd);
					break;
				case TOREAD:
				case CONTINUE:
				default:
					break;
				}
			}
			else if(evlist[i].events & EPOLLOUT)
			{
				heap.UpdateTimer(connections.TimerOfConnect(sockfd));
				ReturnCode ret = connections.Process(sockfd,WRITE);
				switch(ret)
				{
				case TOREAD:
					uint32_t ev = EPOLLIN;
					ModifyFd(_epollfd,sockfd,ev);
					break;
				case TOCLOSE:
					RemoveFd(_epollfd,sockfd);
					connections.RecyleConn(sockfd);
					break;
				case TOWRITE:
				case CONTINUE:
				default:
					break;
				}
			}
			else
			{
				continue;
			}

		}
	}


}




























