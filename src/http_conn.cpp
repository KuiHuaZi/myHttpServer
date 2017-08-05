#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "http_conn.h"
//#include "common_functions.h"
#include"time_heap.h"
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";
const char* doc_root = "/home/amapola/workspace/myHttpServer/html";
/*void http_conn::close_conn( bool real_close )
{
    if( real_close && ( m_sockfd != -1 ) )
    {
        //modfd( m_epollfd, m_sockfd, EPOLLIN );
        removefd( m_epollfd, m_sockfd );
        m_sockfd = -1;
        m_user_count--;
    }
}*/

HttpConn::HttpConn()
{
	_sockfd = -1;
	_read_buf = nullptr;
	_allocated = false;
}
HttpConn::~HttpConn()
{
	if(_allocated)
	{
		delete[]_read_buf;
		delete[]_write_buf;
		delete[]_timer;
	}
}
bool HttpConn::Init( int sockfd,int connect_keep_time,int recv_size,int send_size/*, const sockaddr_in& addr */)
{
    _sockfd = sockfd;
   // _address = addr;
    //for avoid TIME_WAIT
    int error = 0;
    socklen_t len = sizeof( error );
    getsockopt( _sockfd, SOL_SOCKET, SO_ERROR, &error, &len );
    int reuse = 1;
    setsockopt( _sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );
    //end

    if(!_allocated)
    {
    	_read_buf = new char[READ_BUFFER_SIZE];
    	if(!_read_buf)
    	{
    		printf("HttpConn::init new failed!\n");
    		return false;
    	}
    	_read_buf_size = READ_BUFFER_SIZE;
    	_write_buf = new char[WRITE_BUFFER_SIZE];
    	if(!_write_buf)
    	{
    		printf("HttpConn::init new failed!\n");
    		delete[]_read_buf;
    		return false;
    	}
        _write_buf_size = send_size;
        _timer = new Timer(connect_keep_time);
        if(!_timer)
        {
        	printf("HttpConn::init _timer new failed!\n");
        	delete[]_read_buf;
        	delete[]_write_buf;
        	return false;
        }
    }
    init();
    return true;
}

void HttpConn::init()
{
    _check_state = CHECK_STATE_REQUESTLINE;
  //  _request_result = NO_REQUEST;
    _linger = false;

    _method = GET;
    _url = 0;
    _version = 0;
    _content_length = 0;
    _host = 0;
    _start_line = 0;
    _checked_idx = 0;
    _read_idx = 0;
    _write_idx = 0;
    _status = READ;
    memset( _read_buf, '\0', READ_BUFFER_SIZE );
    memset( _write_buf, '\0', WRITE_BUFFER_SIZE );
    memset( _real_file, '\0', FILENAME_LEN );
}

HttpConn::LINE_STATUS HttpConn::parse_line()
{
    char temp;
    for ( ; _checked_idx < _read_idx; ++_checked_idx )
    {
        temp = _read_buf[ _checked_idx ];
        if ( temp == '\r' )
        {
            if ( ( _checked_idx + 1 ) == _read_idx )
            {
                return LINE_OPEN;
            }
            else if ( _read_buf[ _checked_idx + 1 ] == '\n' )
            {
                _read_buf[ _checked_idx++ ] = '\0';
                _read_buf[ _checked_idx++ ] = '\0';
                return LINE_OK;
            }

            return LINE_BAD;
        }
        else if( temp == '\n' )
        {
            if( ( _checked_idx > 1 ) && ( _read_buf[ _checked_idx - 1 ] == '\r' ) )
            {
                _read_buf[ _checked_idx-1 ] = '\0';
                _read_buf[ _checked_idx++ ] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
}

bool HttpConn::read()
{
    if( _read_idx >= READ_BUFFER_SIZE )
    {
        return false;
    }

    int bytes_read = 0;
    bytes_read = recv( _sockfd, _read_buf + _read_idx, READ_BUFFER_SIZE - _read_idx, 0 );
    if ( bytes_read == -1 )
    {
    	if( errno == EAGAIN || errno == EWOULDBLOCK ||errno==EINTR)
        {
            return true;
        }
        return false;
    }
    else if ( bytes_read == 0 )
    {
        return false;
    }
    _read_idx += bytes_read;
    return true;
}
HttpConn::HTTP_CODE HttpConn::parse_request_line( char* text )
{
    _url = strpbrk( text, " \t" );
    if ( ! _url )
    {
        return BAD_REQUEST;
    }
    *_url++ = '\0';

    char* method = text;
    if ( strcasecmp( method, "GET" ) == 0 )
    {
        _method = GET;
    }
    else
    {
        return BAD_REQUEST;
    }

    _url += strspn( _url, " \t" );
    _version = strpbrk( _url, " \t" );
    if ( ! _version )
    {
        return BAD_REQUEST;
    }
    *_version++ = '\0';
    _version += strspn( _version, " \t" );
    if ( strcasecmp( _version, "HTTP/1.1" ) != 0 )
    {
        return BAD_REQUEST;
    }

    if ( strncasecmp( _url, "http://", 7 ) == 0 )
    {
        _url += 7;
        _url = strchr( _url, '/' );
    }
    if ( ! _url || _url[ 0 ] != '/' )
    {
        return BAD_REQUEST;
    }
    printf("url:%s\n",_url);
    _check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::parse_headers( char* text )
{
    if( text[ 0 ] == '\0' )
    {
        if ( _method == HEAD )
        {
            return GET_REQUEST;
        }

        if (_content_length != 0 )
        {
            _check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }

        return GET_REQUEST;
    }
    else if ( strncasecmp( text, "Connection:", 11 ) == 0 )
    {
        text += 11;
        text += strspn( text, " \t" );
        if ( strcasecmp( text, "keep-alive" ) == 0 )
        {
            _linger = true;
        }
    }
    else if ( strncasecmp( text, "Content-Length:", 15 ) == 0 )
    {
        text += 15;
        text += strspn( text, " \t" );
        _content_length = atol( text );
    }
    else if ( strncasecmp( text, "Host:", 5 ) == 0 )
    {
        text += 5;
        text += strspn( text, " \t" );
        _host = text;
    }
    else
    {
        printf( "oop! unknow header %s\n", text );
    }

    return NO_REQUEST;

}

HttpConn::HTTP_CODE HttpConn::parse_content( char* text )
{
    if ( _read_idx >= ( _content_length + _checked_idx ) )
    {
        text[ _content_length ] = '\0';
        return GET_REQUEST;
    }

    return NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char* text = 0;

    while ( ( ( _check_state == CHECK_STATE_CONTENT ) && ( line_status == LINE_OK  ) )
                || ( ( line_status = parse_line() ) == LINE_OK ) )
    {
        text = get_line();
        _start_line = _checked_idx;
        printf( "got 1 http line: %s\n", text );

        switch (_check_state )
        {
            case CHECK_STATE_REQUESTLINE:
            {
                ret = parse_request_line( text );
                if ( ret == BAD_REQUEST )
                {
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER:
            {
                ret = parse_headers( text );
                if ( ret == BAD_REQUEST )
                {
                    return BAD_REQUEST;
                }
                else if ( ret == GET_REQUEST )
                {
                    return do_request();
                }
                break;
            }
            case CHECK_STATE_CONTENT:
            {
                ret = parse_content( text );
                if ( ret == GET_REQUEST )
                {
                    return do_request();
                }
                line_status = LINE_OPEN;
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }

    return NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::do_request()
{
    strcpy( _real_file, doc_root );
    int len = strlen( doc_root );
    strncpy( _real_file + len, _url, FILENAME_LEN - len - 1 );
    printf("file name:%s\n",_real_file);
    if ( stat( _real_file, &_file_stat ) < 0 )
    {
        return NO_RESOURCE;
    }

    if ( ! ( _file_stat.st_mode & S_IROTH ) )
    {
        return FORBIDDEN_REQUEST;
    }

    if ( S_ISDIR( _file_stat.st_mode ) )
    {
        return BAD_REQUEST;
    }

    int fd = open( _real_file, O_RDONLY );
    _file_address = ( char* )mmap( 0, _file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
    close( fd );
    return FILE_REQUEST;
}

void HttpConn::unmap()
{
    if( _file_address )
    {
        munmap( _file_address, _file_stat.st_size );
        _file_address = 0;
    }
}

ReturnCode HttpConn::write()
{
    int temp = 0;
    int bytes_have_send = 0;
    int bytes_to_send = _write_idx;
    if ( bytes_to_send == 0 )
    {
        if( _linger )
        {
        	init();
            return TOREAD;
        }
        else
        {
            return TOCLOSE;
        }
    }

    while( 1 )
    {
        temp = writev( _sockfd, _iv, _iv_count );
        if ( temp <= -1 )
        {
            if( errno == EAGAIN )
            {
                return TOWRITE;
            }
            unmap();
            init();
            return TOCLOSE;
        }

        bytes_to_send -= temp;
        bytes_have_send += temp;
        if ( bytes_to_send <= bytes_have_send )
        {
            unmap();
            if( _linger )
            {
                init();
                return TOREAD;
            }
            else
            {
                return TOCLOSE;
            }
        }
    }
}

bool HttpConn::add_response( const char* format, ... )
{
    if( _write_idx >= WRITE_BUFFER_SIZE )
    {
        return false;
    }
    va_list arg_list;
    va_start( arg_list, format );
    int len = vsnprintf( _write_buf + _write_idx, WRITE_BUFFER_SIZE - 1 - _write_idx, format, arg_list );
    if( len >= ( WRITE_BUFFER_SIZE - 1 - _write_idx ) )
    {
        return false;
    }
    _write_idx += len;
    va_end( arg_list );
    return true;
}

bool HttpConn::add_status_line( int status, const char* title )
{
    return add_response( "%s %d %s\r\n", "HTTP/1.1", status, title );
}

bool HttpConn::add_headers( int content_len )
{
    add_content_length( content_len );
    add_linger();
    add_blank_line();
}

bool HttpConn::add_content_length( int content_len )
{
    return add_response( "Content-Length: %d\r\n", content_len );
}

bool HttpConn::add_linger()
{
    return add_response( "Connection: %s\r\n", ( _linger == true ) ? "keep-alive" : "close" );
}

bool HttpConn::add_blank_line()
{
    return add_response( "%s", "\r\n" );
}

bool HttpConn::add_content( const char* content )
{
    return add_response( "%s", content );
}

bool HttpConn::process_write( HTTP_CODE ret )
{
    switch ( ret )
    {
        case INTERNAL_ERROR:
        {
            add_status_line( 500, error_500_title );
            add_headers( strlen( error_500_form ) );
            if ( ! add_content( error_500_form ) )
            {
                return false;
            }
            break;
        }
        case BAD_REQUEST:
        {
            add_status_line( 400, error_400_title );
            add_headers( strlen( error_400_form ) );
            if ( ! add_content( error_400_form ) )
            {
                return false;
            }
            break;
        }
        case NO_RESOURCE:
        {
            add_status_line( 404, error_404_title );
            add_headers( strlen( error_404_form ) );
            if ( ! add_content( error_404_form ) )
            {
                return false;
            }
            break;
        }
        case FORBIDDEN_REQUEST:
        {
            add_status_line( 403, error_403_title );
            add_headers( strlen( error_403_form ) );
            if ( ! add_content( error_403_form ) )
            {
                return false;
            }
            break;
        }
        case FILE_REQUEST:
        {
            add_status_line( 200, ok_200_title );
            if ( _file_stat.st_size != 0 )
            {
                add_headers( _file_stat.st_size );
                _iv[ 0 ].iov_base = _write_buf;
                _iv[ 0 ].iov_len = _write_idx;
                _iv[ 1 ].iov_base = _file_address;
                _iv[ 1 ].iov_len = _file_stat.st_size;
                _iv_count = 2;
                return true;
            }
            else
            {
                const char* ok_string = "<html><body></body></html>";
                add_headers( strlen( ok_string ) );
                if ( ! add_content( ok_string ) )
                {
                    return false;
                }
            }
            break;
        }
        default:
        {
            return false;
        }
    }

    _iv[ 0 ].iov_base = _write_buf;
    _iv[ 0 ].iov_len = _write_idx;
    _iv_count = 1;
    return true;
}

ReturnCode HttpConn::Process(OptType option)
{
	if(option !=_status)
	{
		return TOCLOSE;
	}
	if(_status == READ)
	{
		bool flag = read();
		if(flag == false)
		{
			return TOCLOSE;
		}
		HTTP_CODE read_ret = process_read();
	    if ( read_ret == NO_REQUEST )
	    {
	        return CONTINUE;
	    }
	    else
	    {
		    bool write_ret = process_write( read_ret );
		    if ( ! write_ret )
		    {
		        return TOCLOSE;
		    }
		    _status = WRITE;
		    ReturnCode code = write();
		    return code;
	    }

	}
	else if(_status == WRITE)
	{
	    ReturnCode code = write();
	    return code;
	}
	return TOCLOSE;

}
#ifdef DEBUG
#include "connect_pool.h"
#include<arpa/inet.h>
#include<assert.h>
#include<string.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<sys/timerfd.h>
//#include"time_heap.h"
int main()
{
	int listen_fd = socket(AF_INET,SOCK_STREAM,0);
	assert(listen_fd>0);
	struct sockaddr_in address;
	memset(&address,0,sizeof(address));
	address.sin_family = AF_INET;
	inet_aton("127.0.0.1",&address.sin_addr);
	address.sin_port = htons(8080);
	const int connect_keep_time = 6000;
	int ret = bind(listen_fd,(struct sockaddr*)&address,sizeof(address));
	assert(ret==0);
	ret = listen(listen_fd,5);
	int epoll_fd = epoll_create(5);
	struct epoll_event evlist[10];
	AddFd(epoll_fd,listen_fd);
	ConnectPool<HttpConn> connect_pool(100);
	TimerHeap timers(100);
	int time_fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
	AddFd(epoll_fd,time_fd);
	uint32_t ev = EPOLLIN|EPOLLET;
	ModifyFd(epoll_fd,time_fd,ev);
	evlist[0].data.fd = 0;//input
	evlist[0].events = EPOLLIN | EPOLLET;
	ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,0,&evlist[0]);
	assert(ret == 0);
	ReturnCode code;
	bool _stop = false;
	while(!_stop)
	{
		int nfds = epoll_wait(epoll_fd,evlist,10,-1);
		printf("epoll_wait:success! nfds%d \n",nfds);
		for(int i = 0; i < nfds; ++i)
		{
			int sockfd = evlist[i].data.fd;
			if((sockfd==listen_fd)&&(evlist[i].events&EPOLLIN))
			{
				printf("new client coming!\n");
				int newfd = accept(listen_fd,nullptr,0);
				if(newfd<0)
				{
					printf("accept failed!\n");
					continue;
				}
				if(!connect_pool.AddConnect(newfd,connect_keep_time))
				{

					close(newfd);
					continue;
				}
				Timer &tmp = connect_pool.TimerOfConnect(newfd);
				timers.InsertTimer(tmp);
				AddFd(epoll_fd,newfd);
				printf("Add new fd:%d\n",newfd);
				if(timers.size()==1)
				{
					struct itimerspec ts;
					memset(&ts,0,sizeof(ts));
					ts.it_value.tv_sec = timers.Min().Expire();
					int flag = TIMER_ABSTIME;
					ret = timerfd_settime(time_fd,flag,&ts,NULL);
					assert(ret == 0);
				}
			}
			else if(sockfd ==time_fd)
			{
				//todo:trick
				int *expire_fd = timers.GetExpireAndSetNewTimer();
				printf("Timer is expired!\n");
				for(int i = 0; expire_fd[i]!=END;++i)
				{
					connect_pool.RecyleConn(expire_fd[i]);
					RemoveFd(epoll_fd,expire_fd[i]);
					printf("Remove fd:%d\n",expire_fd[i]);
				}
				if(timers.IsEmpty())
				{
					continue;
				}
				struct itimerspec ts;
				memset(&ts,0,sizeof(ts));
				ts.it_value.tv_sec = timers.Min().Expire();
				int flag = TIMER_ABSTIME;
				ret = timerfd_settime(time_fd,flag,&ts,NULL);
				assert(ret == 0);
			}
			else if(sockfd == 0)
			{
				printf("stop!\n");
				_stop = true;
			}
			else if(evlist[i].events & EPOLLIN)
			{
				printf("connection:%d have data to read!\n",sockfd);
				code = connect_pool.Process(sockfd,READ);
				printf("update timer!\n");
				Timer &tmp = connect_pool.TimerOfConnect(sockfd);
				tmp.AdjustTimer(connect_keep_time);
				timers.UpdateTimer(tmp);
				uint32_t ev= EPOLLOUT;
				switch(code)
				{
				case TOWRITE:
					printf("READ to WRITE!\n");
					ModifyFd(epoll_fd,sockfd,ev);
					break;
				case TOCLOSE:
					printf("READ to CLOSE\n");
					connect_pool.RecyleConn(sockfd);
					RemoveFd(epoll_fd,sockfd);
					timers.DelTimer(tmp);
					break;
				case TOREAD:
				case CONTINUE:
				default:
					printf("CONTINUE!\n");
					break;
				}
			}
			else if(evlist[i].events & EPOLLOUT)
			{
				printf("connection:%d have data to write!\n",sockfd);
				code = connect_pool.Process(sockfd,WRITE);
				uint32_t ev= EPOLLIN;
				switch(code)
				{
				case TOREAD:
					printf("WRITE to READ\n");
					ModifyFd(epoll_fd,sockfd,ev);
					break;
				case TOCLOSE:
					printf("WRITE to CLOSE\n");
					timers.DelTimer(connect_pool.TimerOfConnect(sockfd));
					connect_pool.RecyleConn(sockfd);
					RemoveFd(epoll_fd,sockfd);
					break;
				case TOWRITE:
				case CONTINUE:
				default:
					printf("CONTINUE!\n");
					break;
				}
			}
			else
			{
				continue;
			}
		}
	}
	close(time_fd);
	close(epoll_fd);
}
#endif


