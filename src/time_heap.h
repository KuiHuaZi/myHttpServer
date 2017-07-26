/*
 * time_heap.h
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#ifndef SRC_TIME_HEAP_H_
#define SRC_TIME_HEAP_H_
#include<time.h>
#include<netinet/in.h>
#include<exception>
#include<iostream>
#include<sys/timerfd.h>
#include<assert.h>
#include<stdio.h>
#include<string.h>
//#define DEBUGHEAP
const int connect_keep_time = 100;
class http_conn;
class Timer
{
public:
	Timer(int delay,http_conn *user):user_conn(user)
	{
		clock_gettime(CLOCK_MONOTONIC,&expire_struct);
		expire_struct.tv_sec+=delay;
		expire = expire_struct.tv_sec;
		cb_func = NULL;
		cb_funct = NULL;
		location_in_heap =-1;
	}
	Timer(int delay)
	{
		clock_gettime(CLOCK_MONOTONIC,&expire_struct);
		expire_struct.tv_sec+=delay;
		expire = expire_struct.tv_sec;
		cb_func = NULL;
		cb_funct = NULL;
		user_conn = NULL;
		location_in_heap =-1;
	}
public:
	struct timespec expire_struct;
	int expire;
	void(*cb_func)(http_conn*);
	void(*cb_funct)();
	http_conn *user_conn;
	int location_in_heap;
};
class TimerHeap
{
public:
	TimerHeap(int cap);
	~TimerHeap();
	void InsertTimer(Timer *t);
	const Timer* Min();
	void DelTimer(Timer *t);
	void PopTimer();
	void UpdateTimer(Timer *t);
	bool IsEmpty();
	int size();
	void Trick();
private:
	void swim(int index);
	void sink(int index);
	void resize(int cap);
	void swap(int i, int j);
public:

	Timer** _heap;
	int _cap;
	int _size;
};



























#endif /* SRC_TIME_HEAP_H_ */
