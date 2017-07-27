/*
 * connect_pool.h
 *
 *  Created on: Jul 26, 2017
 *      Author: amapola
 */

#ifndef SRC_CONNECT_POOL_H_
#define SRC_CONNECT_POOL_H_
#include<map>
#include<set>
#include<assert.h>
#include<sys/epoll.h>
#include"http_conn.h"
using std::map;
using std::set;
enum OptType {READ=0,WRITE,CLOSE};
enum ReturnCode {CONTINUE=0,TOREAD,TOWRITE,TOCLOSE};
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
template<class Conn>
class ConnectPool
{
public:
	ConnectPool(int size,int epollfd);
	~ConnectPool();
	void Process(int connfd,OptType status);
	void CloseConnect(int connfd);
	bool AddConnect(int connfd)
	{
		if(_connect_free.empty())
				{
					if(!resize())
					{
						close(fd);
						printf("ConnectPool::Process new fd %d failed,because resize failed!\n",fd);
						return false;
					}
				}
				Conn* tmp = *_connect_free.begin();
	}
	void RecyleConn(Conn*t);
private:
	bool resize();
private:
	map<int,Conn*> _connect_using;
	set<Conn*> _connect_free;
	int _epollfd;
	int _cap;
};

template<class Conn>
ConnectPool<Conn>::ConnectPool(int size,int epollfd)
{
	_cap = size;
	_epollfd = epollfd;
	Conn*tmp = new Conn[size];
	assert(!tmp);
	for(int i = 0;i<size;i++)
	{
		_connect_free.insert(&tmp[i]);
	}
}
template<class Conn>
void ConnectPool<Conn>::Process(int fd,OptType status)
{
	if(_connect_using.count(fd)==0)
	{
		printf("ConnectPool::Process:no this fd %d in pool!\n",fd);
	}
	else
	{
		Conn*tmp = _connect_using.at(fd);
		ReturnCode ret = tmp->Process(status);
		switch(ret)
		{
		case TOREAD:
			uint32_t event = EPOLLIN;
			ModifyFd(_epollfd,fd,event);
			break;
		case TOWRITE:
			uint32_t event = EPOLLOUT;
			ModifyFd(_epollfd,fd,event);
			break;
		case TOCLOSE:
			RemoveFd(_epollfd,fd);
			RecyleConn(tmp);
			break;
		case CONTINUE:
		default:
			break;

		}
	}
	return;
}


#endif /* SRC_CONNECT_POOL_H_ */
