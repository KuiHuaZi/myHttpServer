/*
 * time_heap.cpp
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#include"time_heap.h"
TimerHeap::TimerHeap(int cap)
{
	_cap = cap ;
	_size = 0 ;
	_heap = new Timer*[_cap+1];
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
	for(int i = 1; i <= _size;++i)
	{
		delete _heap[i];
	}
	delete[]_heap;
}

void TimerHeap::InsertTimer(Timer*t)
{
	if(!t)return;
	if(_size == _cap) resize(_cap * 2);
	_heap[++_size] = t;
	t->location_in_heap = _size;
	swim(_size);
}

const Timer* TimerHeap::Min()
{
	if(IsEmpty())
	{
		return NULL;
	}
	return _heap[1];
}

void TimerHeap::DelTimer(Timer *t)
{
	if(!t)
	{
		return;
	}
//	int index = t->location_in_heap;
	t->cb_func = NULL;
	return;
}

void TimerHeap::PopTimer()
{
	if(IsEmpty())
	{
		return;
	}
	delete _heap[1];
	_heap[1] = _heap[_size--];
	sink(1);
	if((_size<100)&&(_size<_cap/4))
	{
		resize(_cap/2);
	}
	return;
}

void TimerHeap::UpdateTimer(Timer *t)
{
	if(!t)
	{
		return;
	}

	if(_heap[t->location_in_heap] != t)
	{
		return;
	}
	printf("Before Updata expire:%d ,location: %d \n",t->expire,t->location_in_heap);
	clock_gettime(CLOCK_MONOTONIC,&t->expire_struct);
	t->expire_struct.tv_sec+=connect_keep_time;
	t->expire = t->expire_struct.tv_sec;

	sink(t->location_in_heap);
	printf("after Updata expire:%d ,location: %d \n",t->expire,t->location_in_heap);
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

void TimerHeap::swim(int index)
{
	while((index > 1) && (_heap[index]->expire < _heap[index/2]->expire))
	{
		swap(index,index/2);
		index/=2;
	}

}

void TimerHeap::sink(int index)
{
	while(2*index < _size)
	{
		int tmp = index * 2 ;
		if((tmp < _size)&&(_heap[tmp]->expire > _heap[tmp+1]->expire))
		{
			++tmp;
		}
		if(_heap[index]->expire < _heap[tmp]->expire)
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
	_heap[i]->location_in_heap = i;
	_heap[j] = t;
	t->location_in_heap = j;
}
void TimerHeap::resize(int cap)
{
	Timer** temp = new Timer*[cap];
	if(!temp)
	{
		throw std::exception();
	}
	for(int i = 0;i < cap;++i)
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
		if(tmp->expire > cur.tv_sec)
		{
			break;
		}
		if(tmp->cb_func)
		{
			tmp->cb_func(tmp->user_conn);
		}
		if(tmp->cb_funct)
		{
			tmp->cb_funct();
		}
		PopTimer();
		tmp = _heap[1];
	}
}
#ifdef DEBUGGEAP
#include<sys/epoll.h>
#include<errno.h>
#include<stdlib.h>
void cb_func()
{
	struct timespec cur;
	clock_gettime(CLOCK_MONOTONIC,&cur);
	printf("Time now: %d\n",cur.tv_sec);
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
			printf("Begin epoll_wait!\n");
			int nfds = epoll_wait(epoll_fd,evlist,2,-1);
			if(nfds < 0 && errno != EINTR)
			{
				printf("epoll_wait failed \n");
				return ;
			}
			for(int i = 0; i < nfds; ++i)
			{
				if(evlist[i].data.fd == 0)
				{
					printf("read input!\n");
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
							t = new Timer(time);
							t->cb_funct = cb_func;
							heap.InsertTimer(t);
							if(heap._size==1)
							{
								struct itimerspec ts;
								memset(&ts,0,sizeof(ts));
								ts.it_value = heap.Min()->expire_struct;
								int flag = TIMER_ABSTIME;
								int ret = timerfd_settime(time_fd,flag,&ts,NULL);
								assert(ret == 0);
							}
							break;
						case 'p':
							for(int i = 1; i <= heap._size;++i)
							{
								printf("heap[%d]: %d  ",i,heap._heap[i]->expire);
							}
							break;
						case'u':
							int index;
							std::cin>>index;
							if(index<=heap.size())
							{
								for(int i = 1; i <= heap._size;++i)
								{
									printf("heap[%d]: %d  ",i,heap._heap[i]->expire);
								}
								printf("\n");
								heap.UpdateTimer(heap._heap[index]);
								for(int i = 1; i <= heap._size;++i)
								{
									printf("heap[%d]: %d  ",i,heap._heap[i]->expire);
								}

							}
						case'n':
							struct timespec cur;
							clock_gettime(CLOCK_MONOTONIC,&cur);
							printf("Time now: %d\n",cur.tv_sec);
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
					printf("Timer trick!\n");
					heap.Trick();
					struct itimerspec ts;
					memset(&ts,0,sizeof(ts));
					if(heap.IsEmpty())
					{
						continue;
					}
					ts.it_value.tv_sec = heap.Min()->expire;
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
