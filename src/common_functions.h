/*
 * common_functions.h
 *
 *  Created on: Jul 27, 2017
 *      Author: amapola
 */

#ifndef SRC_COMMON_FUNCTIONS_H_
#define SRC_COMMON_FUNCTIONS_H_
#include<stdlib.h>
#include<cstdint>
bool AddFd(int epollfd,int fd);
bool RemoveFd(int epollfd,int fd);
bool ModifyFd(int epollfd,int fd,uint32_t ev);
int SetNonblocking(int fd);
void log(const char *cmd,...);
enum OptType {READ=0,WRITE,CLOSE};
enum ReturnCode {CONTINUE=0,TOREAD,TOWRITE,TOCLOSE};
#endif /* SRC_COMMON_FUNCTIONS_H_ */
