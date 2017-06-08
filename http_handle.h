/*
 * http_handle.h
 *
 *  Created on: Jun 1, 2017
 *      Author: amapola
 */
#include"http_server.h"
#ifndef HTTP_HANDLE_H_
#define HTTP_HANDLE_H_
const int BUFFERSIZE = 4096;
const int MAX_FILENAME_LENGTH = 256;
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
	enum HTTP_CODE{NO_REQUEST,GET_REQUEST,BAD_REQUEST,NO_RESOURCE,FORBIDDEN_REQUEST,FILE_REQUEST,INTERNAL_ERROR,CGI_REQUEST,CLOSED_CONNECTION};
	enum METHOD{GET=0,POST,HEAD,PUT,DELETE,TRACE,OPTION,CONNECT,PATCH};
	void PraseContent(char*buf);
	void GetFileType();
	HTTP_CODE PraseRequestLine();
	HTTP_CODE PraseHeaders();
	LINE_STATUS ReadAndCheckLine();
	HTTP_CODE ProcessRequest();
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
	HTTP_CODE http_code;
	METHOD method;
	int file_size;
};

#endif /* HTTP_HANDLE_H_ */


























