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
#include"http_conn.h"
using std::map;
using std::set;
enum OptType {READ=0,WRITE,CLOSE};
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
		return;
	}
	else
	{
		Conn*tmp = _connect_using.at(fd);
		tmp->Process(status);

	}
}


#endif /* SRC_CONNECT_POOL_H_ */
