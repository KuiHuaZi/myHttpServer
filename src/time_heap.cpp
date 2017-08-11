/*
 * time_heap.cpp
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#include"time_heap.h"
#include<time.h>
#include<netinet/in.h>
#include<exception>
#include<iostream>
#include<sys/timerfd.h>
#include<assert.h>
#include<stdio.h>
#include<string.h>
#include"common_functions.h"
Timer::	Timer(int delay,int fd)
{
	clock_gettime(CLOCK_MONOTONIC,&_expire_struct);
	_expire_struct.tv_sec+=delay;
	_expire = _expire_struct.tv_sec;
	cb_funct = NULL;
	_location_in_heap =-1;
	_fd = fd;
	_delay = delay;
}
void Timer::ResetTimer(int delay,int fd)
{
	clock_gettime(CLOCK_MONOTONIC,&_expire_struct);
	_expire_struct.tv_sec+=delay;
	_expire = _expire_struct.tv_sec;
	cb_funct = NULL;
	_location_in_heap =-1;
	_fd = fd;
	_delay = delay;
}
void Timer::AdjustTimer(int delay)
{
	clock_gettime(CLOCK_MONOTONIC,&_expire_struct);
	_expire_struct.tv_sec+=delay;
	_expire = _expire_struct.tv_sec;
}
TimerHeap::TimerHeap(int cap)
{
	_cap = cap ;
	_size = 0 ;
	_heap = new(std::nothrow) Timer*[_cap+1];
	if(!_heap)
	{
		throw std::exception();
	}
	for(int i = 0; i <= _cap;++i)
	{
	_heap[i] = NULL;
	}
}

TimerHeap::~TimerHeap()
{

	delete[]_heap;
}

void TimerHeap::InsertTimer(Timer&t)
{
	if(_size == _cap) resize(_cap * 2);
	_heap[++_size] = &t;
	t._location_in_heap = _size;
	swim(_size);
}

const Timer& TimerHeap::Min()
{
	return *_heap[1];
}

void TimerHeap::DelTimer(Timer &t)
{
	if(IsEmpty())
	{
		return;
	}
	int index = t._location_in_heap;
	if(_heap[index]!=&t)
		return;
	_heap[index] = _heap[_size--];
	_heap[index]->_location_in_heap = index;
	sink(index);
	return;
}
void TimerHeap::PopTimer()
{
	if(IsEmpty())
	{
		return;
	}
	else if(_size==1)
	{
		_heap[1] = nullptr;
		_size = 0;
		return;
	}
	_heap[1] = _heap[_size--];
	_heap[1]->_location_in_heap = 1;
	sink(1);
	if((_size<100)&&(_size<_cap/4))
	{
		resize(_cap/2);
	}
	return;
}

void TimerHeap::UpdateTimer(Timer &tmp)
{

	Timer*t = &tmp;
	if(_heap[t->_location_in_heap] != t)
	{
		return;
	}
	log("Before Update location: %d \n",t->_location_in_heap);
	sink(t->_location_in_heap);
	log("after Update location: %d \n",t->_location_in_heap);
	return;

}

bool TimerHeap::IsEmpty()
{
	if(_size == 0)
	{
		return true;
	}

	return false;
}

int TimerHeap::size()
{
	return _size;
}

void TimerHeap::Trick()
{
	Timer *tmp = _heap[1];
	struct timespec cur;
	clock_gettime(CLOCK_MONOTONIC,&cur);
	while(!IsEmpty())
	{
		if(!tmp)
		{
			break;
		}
		if(tmp->_expire > cur.tv_sec)
		{
			break;
		}
		if(tmp->cb_funct)
		{
			tmp->cb_funct();
		}
		PopTimer();
		tmp = _heap[1];
	}
}
int* TimerHeap::GetExpireAndSetNewTimer()
{
	Timer *tmp = _heap[1];
	struct timespec cur;
	clock_gettime(CLOCK_MONOTONIC,&cur);
	int i = 0;
	while(!IsEmpty())
	{
		if(!tmp)
		{
			break;
		}
		if(tmp->_expire > cur.tv_sec)
		{
			break;
		}
		if(i>8)
			break;
		_expire_timer[i++] = tmp->_fd;
		PopTimer();
		tmp = _heap[1];
	}
	_expire_timer[i] = END;
	return _expire_timer;
}
void TimerHeap::PrintHeap()
{
	for(int i = 1; i <= _size;++i)
	{
		log("heap[%d]: %ld  ",i,_heap[i]->Expire());
	}
}
void TimerHeap::swim(int index)
{
	while((index > 1) && (_heap[index]->_expire < _heap[index/2]->_expire))
	{
		swap(index,index/2);
		index/=2;
	}

}

void TimerHeap::sink(int index)
{
	while(2*index <= _size)
	{
		int tmp = index * 2 ;
		if((tmp < _size)&&(_heap[tmp]->_expire > _heap[tmp+1]->_expire))
		{
			++tmp;
		}
		if(_heap[index]->_expire < _heap[tmp]->_expire)
		{
			break;
		}
		swap(index,tmp);
		index = tmp;
	}
}

void TimerHeap::swap(int i,int j)
{
	Timer* t = _heap[i];
	_heap[i] = _heap[j];
	_heap[i]->_location_in_heap = i;
	_heap[j] = t;
	t->_location_in_heap = j;
}
void TimerHeap::resize(int cap)
{
	Timer** temp = new(std::nothrow) Timer*[cap+1];
	if(!temp)
	{
		throw std::exception();
	}
	for(int i = 0;i < cap+1;++i)
	{
		temp[i] = NULL;
	}
	_cap = cap;
	for(int i = 1 ; i <= _size;++i)
	{
		temp[i] = _heap[i];
	}
	delete[]_heap;
	_heap = temp;
}

#ifdef DEBUG
#include<sys/epoll.h>
#include<errno.h>
#include<stdlib.h>
void cb_func()
{
	struct timespec cur;
	clock_gettime(CLOCK_MONOTONIC,&cur);
	log("Time now: %ld\n",cur.tv_sec);
}
static void Test()
{
	while(true)
	{
		time_t time;
		int cap;
		//bool stop;
		std::cin>>cap;
		TimerHeap heap(cap);
		int epoll_fd = epoll_create(5);
		struct epoll_event evlist[2];
		evlist[0].data.fd = 0;//input
		evlist[0].events = EPOLLIN | EPOLLET;
		int ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,0,&evlist[0]);
		assert(ret == 0);
		int time_fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
		evlist[1].data.fd = time_fd;
		evlist[1].events = EPOLLIN|EPOLLET;
		ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,time_fd,&evlist[1]);
		assert(ret == 0);
		while(true)
		{
			log("Begin epoll_wait!\n");
			int nfds = epoll_wait(epoll_fd,evlist,2,-1);
			if(nfds < 0 && errno != EINTR)
			{
				log("epoll_wait failed \n");
				return ;
			}
			for(int i = 0; i < nfds; ++i)
			{
				if(evlist[i].data.fd == 0)
				{
					log("read input!\n");
					char tmp;
					std::cin>>tmp;
					while(tmp!='e')
					{
						Timer *t=NULL;
						switch(tmp)
						{
						case '+':
							int time;
							std::cin>>time;
							t = new(std::nothrow) Timer(time);
							t->cb_funct = cb_func;
							heap.InsertTimer(*t);
							if(heap.size()==1)
							{
								struct itimerspec ts;
								memset(&ts,0,sizeof(ts));
								ts.it_value.tv_sec = heap.Min().Expire();
								int flag = TIMER_ABSTIME;
								int ret = timerfd_settime(time_fd,flag,&ts,NULL);
								assert(ret == 0);
							}
							break;
						case 'p':
							heap.PrintHeap();
							break;
						case'u':
							int index;
							std::cin>>index;
							if(index<=heap.size())
							{
								heap.PrintHeap();
								log("\n");
								heap._heap[index]->AdjustTimer(10);
								heap.UpdateTimer(*heap._heap[index]);
								heap.PrintHeap();
							}
							break;
						case'n':
							struct timespec cur;
							clock_gettime(CLOCK_MONOTONIC,&cur);
							log("Time now: %ld\n",cur.tv_sec);
							break;
						case '-':
							break;
						case'q':
							return;
						default:
							break;
						}
					std::cin>>tmp;
					}
				}
				else if(evlist[i].data.fd ==time_fd)
				{
					log("Timer trick!\n");
					heap.Trick();
					struct itimerspec ts;
					memset(&ts,0,sizeof(ts));
					if(heap.IsEmpty())
					{
						continue;
					}
					ts.it_value.tv_sec = heap.Min().Expire();
					int flag = TIMER_ABSTIME;
					int ret = timerfd_settime(time_fd,flag,&ts,NULL);
					assert(ret == 0);
				}
				else
					continue;
			}

		}



	}
}
int main()
{
	Test();
	return 0;
}
#endif
