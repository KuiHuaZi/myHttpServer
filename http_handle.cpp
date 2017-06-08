/*
 * http_handle.cpp
 *
 *  Created on: May 31, 2017
 *      Author: amapola
 */

#include"http_handle.h"
ssize_t RioWriten(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char*)usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)  /* Interrupted by sig handler return */
	    	nwritten = 0;    /* and call write() again */
	    else
		return -1;       /* errno set by write() */
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}
ssize_t RioReadn(int fd,void *usrbuf,size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = (char*)usrbuf;
	while(nleft>0)
	{
		if((nread = read(fd,bufp,nleft))<0)
		{
			if(errno == EINTR)
				nread = 0;
			else
				return -1;

		}
		else if(nread == 0)
		{
			break;
		}
		nleft-=nread;
		bufp+=nread;
	}
	return (n-nleft);
}

const char *error_400_response ="HTTP/1.1 400 Bad Request\r\n";
const char  *error_500_response = "HTTP/1.1 500 Internal Server Error\r\n";
const char *error_404_response = "HTTP/1.1 404 Not Found\r\n";
const char *error_403_response = "HTTP/1.1 403 Forbidden\r\n";
MyHttpHandleClass::MyHttpHandleClass(int confd,sockaddr_in client_addr)
{
	_connfd = confd;
	_client_addr = client_addr;
	memset(recvbuf,'\0',BUFFERSIZE);
	url = NULL;
	check_index = 0;
	read_index = 0;
	line_start = 0;
	line_end = 0;
	is_right_request = false;
	http_code = BAD_REQUEST;//or?
	PraseContent(recvbuf);
	SendResponse();
	close(_connfd);

}
void MyHttpHandleClass::PraseContent(char *buf)
{
	CHECK_STATE check_state = CHECK_STATE_REQUESTLINE;
	LINE_STATUS line;
	while (1) {

		switch (check_state) {
		case CHECK_STATE_REQUESTLINE:
			 line = ReadAndCheckLine();
			if (line == LINE_OK) {
				http_code = PraseRequestLine();
				line_start = line_end+1;
				if (http_code == NO_REQUEST) {
					check_state = CHECK_STATE_HEADER;
					break;
				}
				else{
					return;
				}
			}
			else if (line == LINE_OPEN) {
				break;
			}
			else if (line == LINE_BAD) {
				return;
			}
			break;
		case CHECK_STATE_HEADER:
			line = ReadAndCheckLine();
			if (line == LINE_OK) {
				http_code = PraseHeaders();
				line_start = line_end+1;
				if (http_code == NO_REQUEST) {
					break;
				}
				else if(http_code == GET_REQUEST){
					http_code = ProcessRequest();
					return;
				}
			} else if (line == LINE_OPEN)
				break;
			else if (line == LINE_BAD) {
				return;
			}

			break;

		default:
			return;
		}
	}
}
MyHttpHandleClass::LINE_STATUS MyHttpHandleClass::ReadAndCheckLine()
{
	char temp;
	int recvbuf_left = BUFFERSIZE - read_index;
	if(check_index == read_index)
	{
		long int readlen = read(_connfd,&recvbuf[read_index],recvbuf_left);
		if(readlen==-1)
		{
			if(errno == EINTR)
				return LINE_OPEN;
			else
				return LINE_BAD;
		}
		else if(readlen == 0)//read EOF,indicate client make  a mistake!
		{
			return LINE_BAD;
		}
		printf("recv:\n%s\n",recvbuf);
		read_index+=readlen;
	}
	for (; check_index < read_index-1; check_index++) {
		temp = recvbuf[check_index];
		if (temp == '\r') {
			if((check_index+1)==read_index)
				return LINE_OPEN;
			else if(recvbuf[check_index+1]=='\n')
			{
				line_end = check_index+1;
				check_index+=2;

				return LINE_OK;
			}
			else
				return LINE_BAD;
		}
		else if(temp =='\n')
		{
			if((check_index>1)&&recvbuf[check_index - 1] =='\r')
			{
				line_end = check_index;
				check_index++;
				return LINE_OK;
			}
			return LINE_BAD;
		}

	}
	return LINE_OPEN;
}

MyHttpHandleClass::HTTP_CODE MyHttpHandleClass::PraseRequestLine()
{
	//recvbuf[line_end] = '\0';//change '\n' to '\0'
	recvbuf[line_end-1] ='\0';//change '\r' to '\0'
	if(strpbrk(&recvbuf[line_start]," ")==NULL)
	{
		//strcpy(response_header,"HTTP/1.1 400 Bad Requset\r\n");
		return BAD_REQUEST;
	}
	char *methodp = &recvbuf[line_start];
	char *method_end = strpbrk(methodp," ");
	*method_end = '\0';
	if(strcasecmp(methodp,"GET")==0)
	{
		method = GET;
	}
	else
	{
		printf("we only achieve GET,this method is %s\n",methodp);
		//strcpy(response_header,"HTTP/1.1 500 Internal Server Error\r\n");
		return BAD_REQUEST;
	}
	url=method_end+1;
	char *version = strpbrk(url," ");
	char *url_end = version;
	version++;
	if(strcasecmp(version,"HTTP/1.1")!=0)
	{
		printf("we only support HTTP/1.1\n");
		printf("method:%s\n",version);
		//strcpy(response_header,"HTTP/1.1 500 Internal Server Error\r\n");
		return BAD_REQUEST;
	}
	*url_end = '\0';
	/*if(strncasecmp(url,"http://",7)==0)
	{
		url+=7;
		url = strchr(url,'/');
	}*/
	if(!url||url[0]!='/')
	{
		printf("url:%s\n",url);

		return BAD_REQUEST;
	}
	return NO_REQUEST;
}

MyHttpHandleClass::HTTP_CODE MyHttpHandleClass::PraseHeaders()
{
	if(recvbuf[line_start]=='\r'&&recvbuf[line_start+1]=='\n')
	{
		return GET_REQUEST;
	}
	recvbuf[line_end] = '\0';
	char *temp = &recvbuf[line_start];
	if(strncasecmp(temp,"HOST:",5)==0)
	{
		temp+=5;
		temp+= strspn(temp,"\t");
		printf("the request host is :%s\n",temp);
	}
	else
	{
		printf("we can not handle this header:%s\n",&recvbuf[line_start]);
	}
	return NO_REQUEST;

}
MyHttpHandleClass::HTTP_CODE MyHttpHandleClass::ProcessRequest()
{
		struct stat file_stat;

		if(stat(url+1,&file_stat)<0)
		{
			//strcpy(response_header,"HTTP/1.1 404 Not Found\r\n");
			//RioWriten(_connfd,response_header,strlen(response_header));
			return NO_RESOURCE;
		}
		else if(!(file_stat.st_mode&S_IROTH))
		{
			//strcpy(response_header,"HTTP/1.1 403 Forbidden\r\n");
			//RioWriten(_connfd,response_header,strlen(response_header));
			return FORBIDDEN_REQUEST;
		}
		else if(S_ISDIR(file_stat.st_mode))
		{
			//strcpy(response_header,"HTTP/1.1 400 Bad request\r\n");
			//RioWriten(_connfd,response_header,strlen(response_header));
			return BAD_REQUEST;
		}
		file_size = (int)file_stat.st_size;
		return GET_REQUEST;
}
void MyHttpHandleClass::SendResponse()
{
	long int ret;
	switch(http_code){
	case NO_REQUEST:
		ret = RioWriten(_connfd, (void*)error_500_response, strlen(error_500_response));
		if (ret < 0) {
			printf("write error:%d\n", errno);
		}
		break;
	case GET_REQUEST:
		{
		GetFileType();
		sprintf(response_header,"HTTP/1.1 200 OK\r\n");
		sprintf(response_header,"%sServer: Amapola Web Server\r\n",response_header);
		sprintf(response_header,"%sConnection: close\r\n",response_header);
		sprintf(response_header,"%sContent-length:%d\r\n",response_header,file_size);
		sprintf(response_header,"%sContent-type:%s\r\n\r\n",response_header,file_type);
		ret=RioWriten(_connfd,response_header,strlen(response_header));
		if(ret<0)
		{
			printf("write error:%d\n",errno);
		}
		printf("Response header:\n%s",response_header);
		int filefd = open(url+1,O_RDONLY,0);
		if(sendfile(_connfd,filefd,NULL,(size_t)file_size)<0)
		{
			printf("sendfile error:%d\n",errno);
		}
	/*	char *filep = (char*)mmap(0,file_stat.st_size,PROT_READ,MAP_PRIVATE,filefd,0);
		close(filefd);
		rio_writen(_connfd,filep,file_stat.st_size);
		munmap(filep,file_stat.st_size);*/
	}
		break;
	case BAD_REQUEST:
		ret = RioWriten(_connfd, (void*)error_400_response, strlen(error_400_response));
		if (ret < 0) {
			printf("write error:%d\n", errno);
		}
		break;
	case NO_RESOURCE:
		ret = RioWriten(_connfd, (void*)error_404_response, strlen(error_404_response));
		if (ret < 0) {
			printf("write error:%d\n", errno);
		}
		break;
	case FORBIDDEN_REQUEST:
		ret = RioWriten(_connfd, (void*)error_403_response, strlen(error_403_response));
		if (ret < 0) {
			printf("write error:%d\n", errno);
		}
		break;
	case FILE_REQUEST:
		//todo
		break;
	case INTERNAL_ERROR:
		ret = RioWriten(_connfd, (void*)error_500_response, strlen(error_500_response));
		if (ret < 0) {
			printf("write error:%d\n", errno);
		}
		break;
	case CGI_REQUEST:
		//todo
		break;
	case CLOSED_CONNECTION:
		//todo
		break;
	default:
		ret = RioWriten(_connfd, (void*)error_500_response, strlen(error_500_response));
		if (ret < 0) {
			printf("write error:%d\n", errno);
		}
		break;
	}
	return;

}
void MyHttpHandleClass::GetFileType()
{
	if(strstr(url,".html"))
		strcpy(file_type,"text/html");
	else if(strstr(url,".gif"))
		strcpy(file_type,"image/gif");
	else if(strstr(url,".png"))
		strcpy(file_type,"image/png");
	else if(strstr(url,".jpg"))
		strcpy(file_type,"image/jpeg");
	else
		strcpy(file_type,"text/plain");
}

