//============================================================================
// Name        : HTTPserver.cpp
// Author      : Amapola
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include"http_server.h"
const int LISTENQ = 1024;
int open_listenfd(char *port);
using namespace std;
int main(int argc,char*argv[])
{
	if(argc!=2)
	{
		printf("usage: HTTPserver <port>\n");
		exit(EXIT_FAILURE);
	}
	int listenfd;
	listenfd = open_listenfd(argv[1]);
	if(listenfd ==-1)
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



int open_listenfd(char *port)
{
	struct addrinfo hints,*listp,*p;
	int listenfd,optval=1;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;
	hints.ai_flags|=AI_NUMERICSERV;
	getaddrinfo(NULL,port,&hints,&listp);
	for(p = listp;p;p = p->ai_next)
	{
		if((listenfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))<0)
			continue;
		setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));//eliminates "address already in use" error from bind.
		if(bind(listenfd,p->ai_addr,p->ai_addrlen)==0)
			break;
		close(listenfd);
	}
	freeaddrinfo(listp);
	if(!p)
		return -1;
	if(listen(listenfd,LISTENQ)<0)
	{
		close(listenfd);
		return -1;
	}
	return listenfd;
}
