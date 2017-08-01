/*
 * time_heap.h
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#ifndef SRC_TIME_HEAP_H_
#define SRC_TIME_HEAP_H_
#include<time.h>
#define END -1
class TimerHeap;
class Timer
{
public:
	friend class TimerHeap;
	Timer(int delay,int fd = -1);
	void ResetTimer(int delay,int fd = -1);
	void AdjustTimer(int delay);
	time_t Expire()const//second
	{
		return _expire;
	}
public:
	void(*cb_funct)();
private:
	struct timespec _expire_struct;
	time_t _expire;
	int _fd;
	int _location_in_heap;
	time_t _delay;
};
static void Test();
class TimerHeap
{
public:
	friend void Test();
	TimerHeap(int cap);
	~TimerHeap();
	void InsertTimer(Timer&t);
	const Timer& Min();
	void DelTimer(Timer &t);
	void PopTimer();
	void UpdateTimer(Timer &t);
	bool IsEmpty();
	int size();
	void Trick();
	int *GetExpireAndSetNewTimer();
	void PrintHeap();

private:
	void swim(int index);
	void sink(int index);
	void resize(int cap);
	void swap(int i, int j);
private:

	Timer** _heap;
	int _cap;
	int _size;
	int _expire_timer[10];
};







#endif /* SRC_TIME_HEAP_H_ */
