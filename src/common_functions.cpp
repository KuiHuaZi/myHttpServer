/*
 * common_functions.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: amapola
 */


#include<sys/epoll.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
#include"common_functions.h"
bool AddFd(int epollfd,int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	int ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
	if(ret==-1)
	{
		log("addfd:epoll_ctl failed\n");
		return false;
	}
	ret = SetNonblocking(fd);
	if(ret==-1)
	{
		log("addfd:setnonblocking failed\n");
		return false;
	}
	return true;
}
bool RemoveFd(int epollfd,int fd)
{
	int ret = epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
	if(ret == -1)
	{
		if(errno==EINVAL)
		{
			log("EINVAL errno:%d  %s",errno,strerror(errno));
		}
		else if(errno==EBADF)
		{
			log("EBADF errno:%d  %s",errno,strerror(errno));
		}
		else if(errno == ENOENT)
		{
			log("ENOENT errno:%d  %s",errno,strerror(errno));
		}
		log("removefd:epoll_clt failed!\n");
		return false;
	}
	ret = close(fd);
	if(ret == -1)
	{
		log("removefd:close failed!\n");
		return false;
	}
	return true;

}
bool ModifyFd(int epollfd,int fd,uint32_t ev)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = ev;
	int ret = epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
	if(ret == -1)
	{
		if(errno==EINVAL)
		{
			log("ModifyFd EINVAL errno:%d  %s",errno,strerror(errno));
		}
		else if(errno==EBADF)
		{
			log("ModifyFd EBADF errno:%d  %s",errno,strerror(errno));
		}
		else if(errno == ENOENT)
		{
			log("ModifyFd ENOENT errno:%d  %s",errno,strerror(errno));
		}
		log("ModifyFd :epoll_ctl failed!\n");
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
void log(const char *cmd,...)
{
	//return;
	printf("%s %s",__DATE__,__TIME__);
    va_list vp;
    va_start(vp, cmd);
    int result = vprintf(cmd, vp);
    va_end(vp);
    printf("\n");
    return;
}
