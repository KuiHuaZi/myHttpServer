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
#include<vector>
#include<assert.h>
#include<sys/epoll.h>
//#include"http_conn.h"
using std::map;
using std::set;
using std::vector;
enum OptType {READ=0,WRITE,CLOSE};
enum ReturnCode {CONTINUE=0,TOREAD,TOWRITE,TOCLOSE};
class Timer;
template<class Conn>
class ConnectPool
{
public:
	ConnectPool(int size);
	~ConnectPool();
	ReturnCode Process(int connfd,OptType status);
	void RecyleConn(int connfd);
	bool AddConnect(int connfd);
	Timer*TimerOfConnect(int fd);
private:

	//bool resize();
private:
	map<int,Conn*> _connect_using;
	set<Conn*> _connect_free;
	Conn*_connect_pool;
	int _cap;
};

#endif /* SRC_CONNECT_POOL_H_ */
