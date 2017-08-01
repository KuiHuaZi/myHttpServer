/*
 * echo.h
 *
 *  Created on: Jul 28, 2017
 *      Author: amapola
 */

#ifndef SRC_ECHO_H_
#define SRC_ECHO_H_
#include"connect_pool.h"
class Timer;
const int MAX_BUFFER_SIZE = 100;
class Echo
{
public:
	Echo();
	~Echo();
	bool Init(int connfd,int connect_keep_time,int recv_size=MAX_BUFFER_SIZE,int send_size=MAX_BUFFER_SIZE);
	ReturnCode Process(OptType option);
	Timer&GetTimer();
private:
	ReturnCode readLine();
	ReturnCode writeLine();
private:
	char* _recv_buffer;
	int _recv_buffer_size;
	int _read_index;
	int _check_index;
	int _send_index;
	char* _send_buffer;
	int _send_buffer_size;
	bool _buffer_allocated;
	Timer *_timer;
	OptType _status;
	int _connfd;
	//int _connect_keep_time;
};




#endif /* SRC_ECHO_H_ */
