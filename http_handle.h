/*
 * http_handle.h
 *
 *  Created on: Jun 1, 2017
 *      Author: amapola
 */

#ifndef HTTP_HANDLE_H_
#define HTTP_HANDLE_H_

#include"http_server.h"
static const int BUFFERSIZE = 4096;
static const int MAX_FILENAME_LENGTH = 256;
static const char*my_root_dir ="~/workspace/HTTPserver";
class MyHttpHandleClass
{
public:
	MyHttpHandleClass(int confd,struct sockaddr_in client_addr);
	//~MyHttpHandleClass();
private:
	int _connfd;
	struct sockaddr_in _client_addr;
	int HandleHttpHead(char *head);
	enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0,CHECK_STATE_HEADER};
	enum LINE_STATUS{LINE_OK = 0,LINE_BAD,LINE_OPEN};
	enum HTTP_CODE{NO_REQUEST,GET_REQUEST,BAD_REQUEST};
	void PraseContent(char*buf);
	void GetFileType();
	HTTP_CODE PraseRequestLine();
	HTTP_CODE PraseHeaders();
	LINE_STATUS ReadAndCheckLine();
	char recvbuf[BUFFERSIZE];
	char response_header[BUFFERSIZE];
	char file_type[BUFFERSIZE];
	char *url;
	void SendResponse();
	int check_index ;
	int read_index  ;
	int line_start ;
	int line_end;
	bool is_right_request;

};

#endif /* HTTP_HANDLE_H_ */


























