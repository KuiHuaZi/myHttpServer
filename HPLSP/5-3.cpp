#include<sys/socket.h>
#include<unistd.h>
#include<iostream>
#include<signal.h>
#include<assert.h>
#include<string.h>
#include<netinet/in.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdlib.h>
static bool stop = false;
static void handle_term(int sig)
{
	stop = true;
}
int main(int argc,char *argv[])
{
	signal(SIGTERM,handle_term);
	if(argc<3)
	{
		printf("usage:%s ip_address port_numbert\n",basename(argv[0]));
		return 1;
	}
const char *ip = argv[1];
const int port = atoi(argv[2]);
int fd = socket(PF_INET,SOCK_STREAM,0);
struct sockaddr_in addr;
bzero(&addr,sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port =  htons(port);
inet_pton(AF_INET,ip,&addr.sin_addr);
if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1)
{
	printf("bind error!errno:%d\n",errno);
	return 1;
}
int ret;
ret = listen(fd,5);
assert(ret!=-1);
while(!stop)
{
	sleep(1);
}
close(fd);
return 0;
}
