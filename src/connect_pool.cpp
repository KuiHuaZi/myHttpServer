/*
 * connect_pool.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: amapola
 */
#include "connect_pool.h"
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
	const int connect_keep_time = 60;
	int ret = bind(listen_fd,(struct sockaddr*)&address,sizeof(address));
	assert(ret==0);
	ret = listen(listen_fd,5);
	int epoll_fd = epoll_create(5);
	struct epoll_event evlist[10];
	AddFd(epoll_fd,listen_fd);
	ConnectPool<Echo> connect_pool(100);
	TimerHeap timers(100);
	int time_fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
	AddFd(epoll_fd,time_fd);
	uint32_t ev = EPOLLIN|EPOLLET;
	ModifyFd(epoll_fd,time_fd,ev);
	evlist[0].data.fd = 0;//input
	evlist[0].events = EPOLLIN;
	ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,0,&evlist[0]);
	assert(ret == 0);
	ReturnCode code;
	bool _stop = false;
	while(!_stop)
	{
		int nfds = epoll_wait(epoll_fd,evlist,10,-1);
		log("epoll_wait:success! nfds%d \n",nfds);
		for(int i = 0; i < nfds; ++i)
		{
			int sockfd = evlist[i].data.fd;
			if((sockfd==listen_fd)&&(evlist[i].events&EPOLLIN))
			{
				log("new client coming!\n");
				int newfd = accept(listen_fd,nullptr,0);
				if(newfd<0)
				{
					log("accept failed!\n");
					continue;
				}
				if(!connect_pool.AddConnect(newfd,connect_keep_time))
				{

					close(newfd);
					continue;
				}
				Timer &tmp = connect_pool.TimerOfConnect(newfd);
				timers.InsertTimer(tmp);
				AddFd(epoll_fd,newfd);
				log("Add new fd:%d\n",newfd);
				if(timers.size()==1)
				{
					struct itimerspec ts;
					memset(&ts,0,sizeof(ts));
					ts.it_value.tv_sec = timers.Min().Expire();
					int flag = TIMER_ABSTIME;
					ret = timerfd_settime(time_fd,flag,&ts,NULL);
					assert(ret == 0);
				}
			}
			else if(sockfd ==time_fd)
			{
				//todo:trick
				int *expire_fd = timers.GetExpireAndSetNewTimer();
				log("Timer is expired!\n");
				for(int i = 0; expire_fd[i]!=END;++i)
				{
					connect_pool.RecyleConn(expire_fd[i]);
					RemoveFd(epoll_fd,expire_fd[i]);
					log("Remove fd:%d\n",expire_fd[i]);
				}
				if(timers.IsEmpty())
				{
					continue;
				}
				struct itimerspec ts;
				memset(&ts,0,sizeof(ts));
				ts.it_value.tv_sec = timers.Min().Expire();
				int flag = TIMER_ABSTIME;
				ret = timerfd_settime(time_fd,flag,&ts,NULL);
				assert(ret == 0);
			}
			else if(sockfd == 0)
			{
				log("stop!\n");
				_stop = true;
			}
			else if(evlist[i].events & EPOLLIN)
			{
				log("connection:%d have data to read!\n",sockfd);
				code = connect_pool.Process(sockfd,READ);
				log("update timer!\n");
				Timer &tmp = connect_pool.TimerOfConnect(sockfd);
				tmp.AdjustTimer(connect_keep_time);
				timers.UpdateTimer(tmp);
				uint32_t ev= EPOLLOUT;
				switch(code)
				{
				case TOWRITE:
					log("READ to WRITE!\n");
					ModifyFd(epoll_fd,sockfd,ev);
					break;
				case TOCLOSE:
					log("READ to CLOSE\n");
					connect_pool.RecyleConn(sockfd);
					RemoveFd(epoll_fd,sockfd);
					timers.DelTimer(tmp);
					break;
				case TOREAD:
				case CONTINUE:
				default:
					log("CONTINUE!\n");
					break;
				}
			}
			else if(evlist[i].events & EPOLLOUT)
			{
				log("connection:%d have data to write!\n",sockfd);
				code = connect_pool.Process(sockfd,WRITE);
				uint32_t ev= EPOLLIN;
				switch(code)
				{
				case TOREAD:
					log("WRITE to READ\n");
					ModifyFd(epoll_fd,sockfd,ev);
					break;
				case TOCLOSE:
					log("WRITE to CLOSE\n");
					timers.DelTimer(connect_pool.TimerOfConnect(sockfd));
					connect_pool.RecyleConn(sockfd);
					RemoveFd(epoll_fd,sockfd);

					break;
				case TOWRITE:
				case CONTINUE:
				default:
					log("CONTINUE!\n");
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


