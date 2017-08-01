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

#endif /* SRC_CONNECT_POOL_H_ */
