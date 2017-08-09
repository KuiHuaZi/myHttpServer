#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
//#include "locker.h"
#include"common_functions.h"
class Timer;
class HttpConn
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 4096;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD { GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH };
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

public:
    HttpConn();
    ~HttpConn();

public:
    bool Init( int sockfd,int connect_keep_time,int recv_size=READ_BUFFER_SIZE,int send_size=WRITE_BUFFER_SIZE/*,const sockaddr_in& addr */);
    void close_conn( bool real_close = true );
    ReturnCode Process(OptType option);
    Timer&GetTimer()
    {
    	return *_timer;
    }
private:
    void init();
    HTTP_CODE process_read();
    bool process_write( HTTP_CODE ret );

    HTTP_CODE parse_request_line( char* text );
    HTTP_CODE parse_headers( char* text );
    HTTP_CODE parse_content( char* text );
    HTTP_CODE do_request();
    char* get_line() { return _read_buf + _start_line; }
    LINE_STATUS parse_line();

    bool read();
    ReturnCode write();

    void unmap();
    bool add_response( const char* format, ... );
    bool add_content( const char* content );
    bool add_status_line( int status, const char* title );
    bool add_headers( int content_length );
    bool add_content_length( int content_length );
    bool add_linger();
    bool add_blank_line();

public:
    //static int m_epollfd;
   // static int m_user_count;

private:
    int _sockfd;
    //sockaddr_in _address;
    char *_read_buf;
    int _read_buf_size;
    int _read_idx;
    int _checked_idx;
    int _start_line;
    char *_write_buf;
    int _write_buf_size;
    int _write_idx;

    CHECK_STATE _check_state;
    METHOD _method;
    //HTTP_CODE _request_result;

    char _real_file[ FILENAME_LEN ];
    char* _url;
    char* _version;
    char* _host;
    int _content_length;
    bool _linger;

    char* _file_address;
    struct stat _file_stat;
    struct iovec _iv[2];
    int _iv_count;
    bool _allocated;
    OptType _status;
    Timer *_timer;
    int _file_fd;
};

#endif
