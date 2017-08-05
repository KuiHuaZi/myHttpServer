/*
 * process_pool.cpp
 *
 *  Created on: Jul 21, 2017
 *      Author: amapola
 */
#ifdef DEBUG
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include"echo.h"
int main(int argc,char* argv[])
{
	if(argc<=2)
	{
		log("usage: %s ip_address port_number\n",basename(argv[0]));
		return 1;
	}
	const char*ip = argv[1];
	int port = atoi(argv[2]);

	int listenfd = socket(AF_INET,SOCK_STREAM,0);
	assert(listenfd>=0);
	int ret = 0;
	struct sockaddr_in address;
	bzero(&address,sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET,ip,&address.sin_addr);
	address.sin_port = htons(port);
	ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
	assert(ret==0);
	ret = listen(listenfd,5);
	assert(ret==0);
	ProcessPool<Echo>&pool = ProcessPool<Echo>::instance(listenfd,2);
	pool.Run();

	close(listenfd);
	return 0;
}

#endif

