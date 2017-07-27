/*
 * process_poll.h
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#ifndef SRC_PROCESS_POOL_H_
#define SRC_PROCESS_POOL_H_
#include<unistd.h>
#include<pthread.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<signal.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/types.h>
class Process
{
public:
	Process():m_pid(-1){}
public:
	pid_t m_pid;
	int m_pipefd[2];//use for connect with main process
};
template< class T>
class ProcessPool
{
public:
	static ProcessPool<T>& instance(int listenfd,int process_number)
	{
		static ProcessPool<T>processpool(listenfd,process_number);
		return processpool;
	}
	~ProcessPool()
	{
		delete[]_sub_process;
	}
	void Run();
private:
	ProcessPool(int listenfd,int process_number = 8);
	ProcessPool(ProcessPool<T> const&);
	ProcessPool<T>& operator=(ProcessPool<T> const&);
	void RunChild();
	void RunParent();
	void SetupSigPipe();
private:
	int _listenfd;
	int _process_number;
	static const int MAX_PROCESS_NUMBER = 16;
	static const int USER_PER_PROCESS = 65536;
	static const int MAX_EVENT_NUMBER = 10000;
	static const int CAP_OF_TIMERHEAP = 100;
	static const int NUMBER_OF_CONNECTION = 100;
	const int CONNECT_KEEP_TIME = 100;
	bool _stop;
	Process *_sub_process;
	int _epollfd;
	int _index;
	int _connect_number;

};


#endif /* SRC_PROCESS_POOL_H_ */
