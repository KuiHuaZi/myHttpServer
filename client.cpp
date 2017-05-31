/*
 * client.c
 *
 *  Created on: May 30, 2017
 *      Author: amapola
 *
 */
#define MAXLEN 16384
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<wait.h>
int main(int argc,char *argv[])
{
	int fd,numchilder,loops,numrequest,port;
	pid_t pid;
	struct sockaddr_in server;
	char request[MAXLEN],reply[MAXLEN];
	if(argc < 6)
	{
		printf("usage:client <IPaddr> <port> <#children> <#loops/child> <#bytes/request>");
		return 1;
	}
	numchilder = atoi(argv[3]);
	loops = atoi(argv[4]);
	numrequest = atoi(argv[5]);
	inet_aton(argv[1],&server.sin_addr);
	port = atoi(argv[2]);
	server.sin_port = htons(port);
	snprintf(request,sizeof(request),"%d\n",numrequest);
	for(int i = 0;i<numchilder;i++)
	{
		if((pid =fork())==0)
		{
			for(int j =0;j<loops;j++)
			{
			fd = socket(PF_INET,SOCK_STREAM,0);
			connect(fd,(struct sockaddr*)&server,sizeof(struct sockaddr_in));
			send(fd,(void*)request,strlen(request),0);
			//recv(fd,(void*)reply,MAXLEN,0);
			if(recv(fd,(void*)reply,MAXLEN,0)==-1)
			{
				printf("error in recv \n");
				return 1;
			}
			close(fd);
			}
			printf("child %d done \n",i);
			exit(0);
		}
	}
	while(wait(NULL)>0)
		;
	if(errno !=ECHILD)
	{
		printf("wait error \n");
		return 1;
	}
	exit(0);
}



