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
const int connect_keep_time = 20;
class http_conn;
class Timer
{
public:
	Timer(int delay,http_conn user):user_conn(user)
	{
		expire = time(NULL) + delay;
	}
private:
	time_t expire;
	void(*cb_func)(http_conn&);
	http_conn &user_conn;
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
private:

	Timer** _heap;
	int _cap;
	int _size;
};





























#endif /* SRC_TIME_HEAP_H_ */
