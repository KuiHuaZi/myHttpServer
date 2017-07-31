/*
 * connect_pool.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: amapola
 */
#include<assert.h>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<utility>
#include "connect_pool.h"
#include "common_functions.h"
using std::pair;
template<class Conn>
ConnectPool<Conn>::ConnectPool(int size)
{
	_cap = size;
	_connect_pool = new Conn[size];
	for(int i = 0;i<size;i++)
	{
		_connect_free.insert(&_connect_pool[i]);
	}
}
template<class Conn>
ConnectPool<Conn>::~ConnectPool()
{
	delete[]_connect_pool;

}
template<class Conn>
ReturnCode ConnectPool<Conn>::Process(int fd,OptType status)
{
	ReturnCode ret;
	if(_connect_using.count(fd)==0)
	{
		printf("ConnectPool::Process:no this fd %d in pool!\n",fd);
		return TOCLOSE;
	}
	else if(status == CLOSE)
	{
		//RecyleConn(fd);
		return TOCLOSE;
	}
	else
	{
		Conn*tmp = _connect_using.at(fd);
		 ret = tmp->Process(status);
		if(ret == TOCLOSE)
		{
			//RecyleConn(fd);
			return TOCLOSE;
		}
	}
	return ret;
}
template<class Conn>
bool ConnectPool<Conn>::AddConnect(int connfd,int connect_keep_time)
{
	if(_connect_free.empty())
	{
		printf("ConnectPool::add new fd %d failed,because no more free connection!\n",connfd);
		return false;
	}
	Conn* tmp = *_connect_free.begin();
	if(!tmp->Init(connfd,connect_keep_time))
	{
		printf("ConnectPool::add new fd %d failed,because connect init failed!\n",connfd);
		return false;
	}
	_connect_free.erase(tmp);
	_connect_using.insert(pair<int,Conn*>(connfd,tmp));
	return true;
}
template<class Conn>
void ConnectPool<Conn>::RecyleConn(int connfd)
{
	if(_connect_using.count(connfd)==0)
	{
		return;
	}
	Conn*tmp = _connect_using.at(connfd);
	_connect_using.erase(connfd);
	_connect_free.insert(tmp);
	return;

}
template<class Conn>
Timer* ConnectPool<Conn>::TimerOfConnect(int connfd)
{
	if(_connect_using.count(connfd)==0)
	{
		return nullptr;
	}
	Conn* tmp = _connect_using.at(connfd);
	return tmp->GetTimer();
}
#ifdef DEBUG
#include<arpa/inet.h>
#include<assert.h>
#include<string.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<sys/timerfd.h>
#include"echo.h"
#include"time_heap.h"
int main()
{
	int listen_fd = socket(AF_INET,SOCK_STREAM,0);
	assert(listen_fd>0);
	struct sockaddr_in address;
	memset(&address,0,sizeof(address));
	address.sin_family = AF_INET;
	inet_aton("127.0.0.1",&address.sin_addr);
	address.sin_port = htons(8080);
	const int connect_keep_time = 10;
	int ret = bind(listen_fd,(struct sockaddr*)&address,sizeof(address));
	assert(ret==0);
	ret = listen(listen_fd,5);
	int epoll_fd = epoll_create(5);
	struct epoll_event evlist[10];
	AddFd(epoll_fd,listen_fd);
	ConnectPool<Echo> connect_pool(2);
	TimerHeap timers(2);
	int time_fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
	AddFd(epoll_fd,time_fd);
	uint32_t ev = EPOLLIN|EPOLLET;
	ModifyFd(epoll_fd,time_fd,ev);
	evlist[0].data.fd = 0;//input
	evlist[0].events = EPOLLIN | EPOLLET;
	ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,0,&evlist[0]);
	assert(ret == 0);
	ReturnCode code;
	bool _stop = false;
	while(!_stop)
	{
		int nfds = epoll_wait(epoll_fd,evlist,10,-1);
		printf("epoll_wait:success! nfds%d \n",nfds);
		for(int i = 0; i < nfds; ++i)
		{
			int sockfd = evlist[i].data.fd;
			if((sockfd==listen_fd)&&(evlist[i].events&EPOLLIN))
			{
				printf("new client coming!\n");
				int newfd = accept(listen_fd,nullptr,0);
				if(newfd<0)
				{
					printf("accept failed!\n");
					continue;
				}
				if(!connect_pool.AddConnect(newfd,connect_keep_time))
				{

					close(newfd);
					continue;
				}
				Timer *tmp = connect_pool.TimerOfConnect(newfd);
				timers.InsertTimer(tmp);
				AddFd(epoll_fd,newfd);
				printf("Add new fd:%d\n",newfd);
				if(timers.size()==1)
				{
					struct itimerspec ts;
					memset(&ts,0,sizeof(ts));
					ts.it_value.tv_sec = timers.Min()->expire;
					int flag = TIMER_ABSTIME;
					ret = timerfd_settime(time_fd,flag,&ts,NULL);
					assert(ret == 0);
				}
			}
			else if(sockfd ==time_fd)
			{
				//todo:trick
				int *expire_fd = timers.GetExpireAndSetNewTimer();
				printf("Timer is expired!\n");
				for(int i = 0; expire_fd[i]!=END;++i)
				{
					connect_pool.RecyleConn(expire_fd[i]);
					RemoveFd(epoll_fd,expire_fd[i]);
					printf("Remove fd:%d",expire_fd[i]);
				}
				if(timers.IsEmpty())
				{
					continue;
				}
				struct itimerspec ts;
				memset(&ts,0,sizeof(ts));
				ts.it_value.tv_sec = timers.Min()->expire;
				int flag = TIMER_ABSTIME;
				ret = timerfd_settime(time_fd,flag,&ts,NULL);
				assert(ret == 0);
			}
			else if(sockfd == 0)
			{
				printf("stop!\n");
				_stop = true;
			}
			else if(evlist[i].events & EPOLLIN)
			{
				printf("connection:%d have data to read!\n",sockfd);
				code = connect_pool.Process(sockfd,READ);
				printf("update timer!\n");
				Timer *tmp = connect_pool.TimerOfConnect(sockfd);
				tmp->AdjustTimer(connect_keep_time);
				timers.UpdateTimer(tmp);
				uint32_t ev= EPOLLOUT;
				switch(code)
				{
				case TOWRITE:
					printf("READ to WRITE!\n");
					ModifyFd(epoll_fd,sockfd,ev);
					break;
				case TOCLOSE:
					printf("READ to CLOSE\n");
					connect_pool.RecyleConn(sockfd);
					RemoveFd(epoll_fd,sockfd);
					timers.DelTimer(tmp);
					break;
				case TOREAD:
				case CONTINUE:
				default:
					printf("CONTINUE!\n");
					break;
				}
			}
			else if(evlist[i].events & EPOLLOUT)
			{
				printf("connection:%d have data to write!\n",sockfd);
				code = connect_pool.Process(sockfd,WRITE);
				Timer *tmp = connect_pool.TimerOfConnect(sockfd);
				tmp->AdjustTimer(connect_keep_time);
				timers.UpdateTimer(tmp);
				uint32_t ev= EPOLLIN;
				switch(code)
				{
				case TOREAD:
					printf("WRITE to READ\n");
					ModifyFd(epoll_fd,sockfd,ev);
					break;
				case TOCLOSE:
					printf("WRITE to CLOSE\n");
					connect_pool.RecyleConn(sockfd);
					RemoveFd(epoll_fd,sockfd);
					timers.DelTimer(tmp);
					break;
				case TOWRITE:
				case CONTINUE:
				default:
					printf("CONTINUE!\n");
					break;
				}
			}
			else
			{
				continue;
			}
		}
	}
	close(time_fd);
	close(epoll_fd);
}
#endif


