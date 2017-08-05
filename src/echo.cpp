/*
 * echo.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: amapola
 */
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include"echo.h"
#include"time_heap.h"
Echo::Echo()
{
	_connfd = -1;
	_buffer_allocated = false;
	_recv_buffer = nullptr;
	_recv_buffer_size = 0;
	//_send_buffer = nullptr;
	_send_buffer_size = 0;
	_status = CLOSE;
	_timer = nullptr;
	_read_index = 0;
	_check_index =0;
	_send_index = 0;
}
Echo::~Echo()
{
	if(_buffer_allocated)
	{
		delete[]_recv_buffer;
		//delete[]_send_buffer;
		delete _timer;
	}
	if(_status!=CLOSE)
	{
		close(_connfd);
	}
}
bool Echo::Init(int connfd,int connect_keep_time,int recv_size,int send_size)
{
	_connfd = connfd;
	_status = READ;
	//_connect_keep_time = connect_keep_time;
	if(!_buffer_allocated)
	{
		_recv_buffer = new char[recv_size+1];
		if(!_recv_buffer)
		{
			printf("Echo::Init() new failed!\n");
			return false;
		}
		_recv_buffer_size = recv_size;
	/*	_send_buffer = new char[send_size+1];
		if(!_send_buffer)
		{
			delete[]_recv_buffer;
			_recv_buffer = nullptr;
			printf("Echo::Init() new failed!\n");
			return false;
		}
		*/
		_send_buffer_size = send_size;
		_timer = new Timer(connect_keep_time,_connfd);
		if(!_timer)
		{
			delete[]_recv_buffer;
			_recv_buffer = nullptr;
			//delete[]_send_buffer;
			//_send_buffer = nullptr;
			printf("Echo::Init() new failed!\n");
			return false;
		}
		_buffer_allocated = true;
	}
	else
	{
		_read_index = _check_index = _send_index=0;
		memset(_recv_buffer,0,recv_size+1);
		//memset(_send_buffer,0,send_size+1);
		_timer->ResetTimer(connect_keep_time,_connfd);
	}

	return true;

}
Timer& Echo::GetTimer()
{
	return *_timer;
}
ReturnCode Echo::Process(OptType option)
{
	if(option!=_status)
	{
		_status = CLOSE;
		return TOCLOSE;
	}
	ReturnCode ret;
	switch(option)
	{
	case READ:
		ret = readLine();
		break;
	case WRITE:
		ret = writeLine();
		break;
	default:
		ret = TOCLOSE;
		break;
	}
	if(ret == TOREAD)
	{
		_status = READ;
	}
	else if(ret == TOWRITE)
	{
		_status = WRITE;
	}
	else if(ret == TOCLOSE)
	{
		_status = CLOSE;
	}
	return ret;

}
ReturnCode Echo::readLine()
{
	int buffer_left = _recv_buffer_size - _read_index;
	if(buffer_left <= 0)
	{
		return TOWRITE;
	}
	int num_read = read(_connfd,_recv_buffer+_read_index,buffer_left);
	if(num_read == 0&&_check_index==_read_index)
	{
		printf("Echo::readLine client closed!\n");
		return TOCLOSE;
	}
	else if(num_read<0)
	{
		if((errno!=EINTR)&&(errno!=EAGAIN))
		{
			printf("Echo::readLine read failed!\n");
			printf("errno %d :\t\t%s\n",errno,strerror(errno));
			return TOCLOSE;
		}
		else
		{
			return CONTINUE;
		}
	}
	_read_index += num_read;
	if(_read_index >=_recv_buffer_size)
	{
		_check_index = _recv_buffer_size;
		return TOWRITE;
	}
	while(_check_index < _read_index)
	{
		if(_recv_buffer[_check_index++]=='\n')
		{
			return TOWRITE;
		}
	}
	return CONTINUE;
}
ReturnCode Echo::writeLine()
{
	int num_to_write = _check_index - _send_index;
	if(num_to_write == 0)
	{
		_check_index = _send_index = _read_index = 0;
		return TOREAD;
	}
	int num_write = write(_connfd,_recv_buffer+_send_index,num_to_write);
	if(num_write == 0)
	{
		printf("Echo::writeLine write failed!\n");
		return TOCLOSE;
	}
	else if(num_write < 0)
	{
		if((errno!=EINTR)&&(errno!=EAGAIN))
		{
			printf("Echo::readLine read failed!]n");
			return TOCLOSE;
		}
		else
		{
			return CONTINUE;
		}
	}
	_send_index += num_write;
	if(_send_index < _check_index)
	{
		return CONTINUE;
	}
	if(_check_index==_read_index)
	{
		_check_index = _send_index = _read_index = 0;
	}
	return TOREAD;

}
#ifdef DEBUG
#include<sys/socket.h>
#include<arpa/inet.h>
#include<assert.h>
#include<string.h>
#include<fcntl.h>
int main()
{
	int listenfd = socket(AF_INET,SOCK_STREAM,0);
	assert(listenfd>0);
	struct sockaddr_in address;
	memset(&address,0,sizeof(address));
	address.sin_family = AF_INET;
	inet_aton("127.0.0.1",&address.sin_addr);
	address.sin_port = htons(8080);
	int ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
	assert(ret==0);
	ret = listen(listenfd,5);
	Echo test;
	const int conneck_keep_time = 100;
	while(true)
	{
		int newfd = accept(listenfd,nullptr,0);
		assert(newfd!=-1);
		printf("accept new client!\n");
		int flag = fcntl(newfd,F_GETFL);
		assert(flag!=-1);
		flag = flag | O_NONBLOCK;
		int ret = fcntl(newfd,F_SETFL,flag);
		assert(ret!=-1);
		//return flag;

		test.Init(newfd,conneck_keep_time);
		ReturnCode code;
		while(true)
		{
			while((code=test.Process(READ))==CONTINUE);
			if(code == TOCLOSE)
			{
				close(newfd);
				break;
			}
			if(code ==TOWRITE)
			{
				while((code = test.Process(WRITE))==CONTINUE);
				if(code == TOCLOSE)
				{
					close(newfd);
					break;
				}
				if(code == TOREAD)
				{
					continue;
				}
			}
		}
	}

}
#endif

