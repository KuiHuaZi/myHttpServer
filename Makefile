CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =  http_server.o http_handle.o		

LIBS =

TARGET = http 

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
#http :http_server.o http_handle.o
#	g++ -o http http_server.o http_handle.o
#http_server.o: http_server.cpp http_server.h
#	g++ -o http_server.cpp
#http_handle.o:http_handle.cpp http_server.h http_handle.h
#	g++ -o http_handle.cpp
#all:http
#clean:
#	rm http http_server.o http_handle.o