/*
 * common_functions.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: amapola
 */

#include"common_functions.h"
#include<sys/epoll.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
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
bool ModifyFd(int epollfd,int fd,uint32_t ev)
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
int SetNonblocking(int fd)
{
	int flag = fcntl(fd,F_GETFL);
	assert(flag!=-1);
	flag = flag | O_NONBLOCK;
	int ret = fcntl(fd,F_SETFL,flag);
	assert(ret!=-1);
	return flag;
}

