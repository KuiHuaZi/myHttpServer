TARGET = HttpServer
OBJECTS = main.o common_functions.o process_pool.o connect_pool.o http_conn.o time_heap.o
$(TARGET): $(OBJECTS)
	g++ -std=c++11 -g -o  $(TARGET) $(OBJECTS)
main.o: process_pool.h http_conn.h
	g++ -std=c++11 -g -c main.cpp
common_functions.o:common_functions.h
	g++ -std=c++11 -g -c common_functions.cpp
process_pool.o:process_pool.h common_functions.h connect_pool.h time_heap.h
	g++ -std=c++11 -g -c process_pool.cpp
connect_pool.o:connect_pool.h time_heap.h
	g++ -std=c++11 -g -c connect_pool.cpp
http_conn.o:http_conn.h
	g++ -std=c++11 -g -c http_conn.cpp
time_heap.o:time_heap.h
	g++ -std=c++11 -g -c time_heap.cpp
.PHONY:clean
clean:
	-rm *.o $(TARGET)