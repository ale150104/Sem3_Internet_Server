#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include "Cookie.h"

/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */

//2MB max size
enum { MSG_MAX = 2097152};



typedef struct __attribute__((packed))
{
	char Method[5];
	char Url[512];
	char Version[9];  
	int contentLength;
	char contentType[512];
	char Host[512];
	char Referer[512];
	char Connection[10];    

} HTTPREQUEST;


int networkReceive(int fd, char *buffer);
int networkSend(int fd, const char *buffer);


#endif
