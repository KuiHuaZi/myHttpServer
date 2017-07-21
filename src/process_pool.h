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
	void run();
private:
	ProcessPool(int listenfd,int process_number = 8);
	ProcessPool(ProcessPool<T> const&);
	ProcessPool<T>& operator=(ProcessPool<T> const&);
	void run_child();
	void run_parent();
	void setup_sig_pipe();
private:
	int _listenfd;
	int _process_number;
	static const int MAX_PROCESS_NUMBER = 16;
	static const int USER_PER_PROCESS = 65536;
	static const int MAX_EVENT_NUMBER = 10000;
	bool _stop;
	Process *_sub_process;
	int _epollfd;
	int _index;

};


#endif /* SRC_PROCESS_POOL_H_ */
