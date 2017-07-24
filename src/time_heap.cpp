/*
 * time_heap.cpp
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */

#include"time_heap.h"
TimerHeap::TimerHeap(int cap)
{
	_cap = cap + 1;
	_size = 0 ;
	_heap = new Timer*[_cap];
	if(!_heap)
	{
		throw std::exception();
	}
	for(int i = 0; i < _cap;++i)
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

	t->expire = time(NULL) + connect_keep_time;

	sink(t->location_in_heap);

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
	_heap[i]->location_in_heap = j;
	_heap[j] = t;
	t->location_in_heap = i;
}
void TimerHeap::resize(int cap)
{
	Timer** temp = new Timer**[cap];
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
	time_t cur = time(NULL);
	while(!IsEmpty())
	{
		if(!tmp)
		{
			break;
		}
		if(tmp->expire > cur)
		{
			break;
		}
		if(tmp->cb_func)
		{
			tmp->cb_func(tmp->user_conn);
		}
		PopTimer();
		tmp = _heap[1];
	}
}

static void Test()
{
	while(true)
	{

	}
}


