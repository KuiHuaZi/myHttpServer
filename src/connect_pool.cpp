/*
 * connect_pool.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: amapola
 */
#include "connect_pool.h"
#include "common_functions.h"
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
	else
	{
		Conn*tmp = _connect_using.at(fd);
		 ret = tmp->Process(status);
		if(ret == TOCLOSE)
		{
			RecyleConn(fd);
			return TOCLOSE;
		}
	}
	return ret;
}
template<class Conn>
bool ConnectPool<Conn>::AddConnect(int connfd)
{
	if(_connect_free.empty())
	{
		//if(!resize())
		//{
			close(connfd);
			printf("ConnectPool::add new fd %d failed,because no more free connection!\n",connfd);
			return false;
		//}
	}
	Conn* tmp = *_connect_free.begin();
	if(!tmp->Init(connfd))
	{
		close(connfd);
		printf("ConnectPool::add new fd %d failed,because connect init failed!\n",connfd);
		return false;
	}
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
		return NULL;
	}
	Conn* tmp = _connect_using.at(connfd);
	return tmp->GetTimer();
}



