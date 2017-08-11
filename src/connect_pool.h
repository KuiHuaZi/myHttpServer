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
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<utility>
#include "common_functions.h"
using std::pair;
using std::map;
using std::set;
class Timer;
template<class Conn>
class ConnectPool
{
public:
	ConnectPool(int size);
	~ConnectPool();
	bool AddConnect(int connfd,int connect_keep_time);
	void RecyleConn(int connfd);
	ReturnCode Process(int connfd,OptType status);
	bool IsContainConnection(int connfd)
	{
		return _connect_using.count(connfd);
	}
	Timer&TimerOfConnect(int fd);
	int NumberOfUsingConnect()
	{
		return _connect_using.size();
	}
	int NumberOfFreeConnect()
	{
		return _connect_free.size();
	}
private:
	map<int,Conn*> _connect_using;
	set<Conn*> _connect_free;
	Conn*_connect_pool;
	int _cap;
};
/*
 * connect_pool.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: amapola
 */

template<class Conn>
ConnectPool<Conn>::ConnectPool(int size)
{
	_cap = size;
	_connect_pool = new(std::nothrow) Conn[size];
	if(!_connect_pool)
	{
		_exit(1);
	}
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
bool ConnectPool<Conn>::AddConnect(int connfd,int connect_keep_time)
{
	if(_connect_free.empty())
	{
		log("ConnectPool::add new fd %d failed,because no more free connection!\n",connfd);
		return false;
	}
	Conn* tmp = *_connect_free.begin();
	if(!tmp->Init(connfd,connect_keep_time))
	{
		log("ConnectPool::add new fd %d failed,because connect init failed!\n",connfd);
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
ReturnCode ConnectPool<Conn>::Process(int fd,OptType status)
{
	ReturnCode ret;
	if(_connect_using.count(fd)==0)
	{
		log("ConnectPool::Process:no this fd %d in pool!\n",fd);
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
	}
	return ret;
}
template<class Conn>
Timer& ConnectPool<Conn>::TimerOfConnect(int connfd)
{
	Conn* tmp = _connect_using.at(connfd);
	log("ConnectPool::TimerOfConnect:Conn tmp:%d\n",tmp);
	return tmp->GetTimer();
}
#endif /* SRC_CONNECT_POOL_H_ */
