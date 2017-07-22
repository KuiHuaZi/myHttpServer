/*
 * process_poll.h
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#ifndef SRC_PROCESS_POOL_H_
#define SRC_PROCESS_POOL_H_
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
class Process
{
public:
	Process():m_pid(-1){}
public:
	pid_t m_pid;
	int m_pipefd[2];//use for connect with main process
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
private:
	int _listenfd;
	int _process_number;
	static const int MAX_PROCESS_NUMBER = 16;
	static const int USER_PER_PROCESS = 65536;
	static const int MAX_EVENT_NUMBER = 10000;
	bool _stop;
	Process *_sub_process;
	int _epollfd;
	int _index;

};
template<class T>
ProcessPool<T>::ProcessPool(int listenfd,int process_number)
:_listenfd(listenfd),_process_number(process_number),_stop(false),_index(-1),_epollfd(-1)
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
			break;
		}
		else
		{
			close(_sub_process[i].m_pipefd[1]);//in parent process
			continue;
		}
	}

}
static int sig_pipefd[2];
int SetNonblocking(int fd)
{
	int flag = fcntl(fd,F_GETFL);
	assert(flag!=-1);
	flag = flag | O_NONBLOCK;
	int ret = fcntl(fd,F_SETFL,flag);
	assert(ret!=-1);
	return flag;
}
bool AddFd(int epollfd,int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	int ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
	if(ret==-1)
	{
		printf("addfd:epoll_ctl failed\n");
		return false;
	}
	ret = setnonblocking(fd);
	if(ret==-1)
	{
		printf("addfd:setnonblocking failed\n");
		return false;
	}
	return true;
}
bool RemoveFd(int epollfd,int fd)
{
	int ret = epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
	if(ret == -1)
	{
		printf("removefd:epoll_clt failed!\n");
		return false;
	}
	ret = close(fd);
	if(ret == -1)
	{
		printf("removefd:close failed!\n");
		return false;
	}
	return true;

}
bool ModifyFd(int epollfd,int fd,int ev)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = ev;
	int ret = epoll_ctl(epollfd,fd,EPOLL_CTL_MOD,&event);
	if(ret == -1)
	{
		printf("modfd:epoll_ctl failed!\n");
		return false;
	}
	return true;
}
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


#endif /* SRC_PROCESS_POOL_H_ */
