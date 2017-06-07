//============================================================================
// Name        : HTTPserver.cpp
// Author      : Amapola
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include"http_server.h"
using namespace std;
int main(int argc,char*argv[])
{
	if(argc!=2)
	{
		printf("usage: HTTPserver <port>\n");
		exit(EXIT_FAILURE);
	}
	int listenfd,port;
	struct sockaddr_in server_addr;
	port = atoi(argv[1]);
	inet_aton("127.0.0.1",&server_addr.sin_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	listenfd = socket(PF_INET,SOCK_STREAM,0);
	if(listenfd ==-1)
	{
		printf("listen failed!\n");
		exit(EXIT_FAILURE);
	}
	if(bind(listenfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr_in))==-1)
	{
		printf("bind failed!\n");
		exit(EXIT_FAILURE);
	}
	if(listen(listenfd,5)==-1)
	{
		printf("listen failed!\n");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		struct sockaddr_in client_addr;
		socklen_t client_addr_length = sizeof(client_addr);
		int connfd = accept(listenfd,(struct sockaddr*)&client_addr,&client_addr_length);
		if(connfd == -1)
		{
			printf("accept error!\n");
			exit(EXIT_FAILURE);
		}
		MyHttpHandleClass http_handle(connfd,client_addr);
		close(connfd);
	}

	return 1;
}
