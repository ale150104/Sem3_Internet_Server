#ifndef COOKIE_H
#define COOKIE_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>


typedef struct InternCookie
{
	struct InternCookie *prev;
	struct InternCookie *next;
	pthread_t thread;	//thread ID of the client thread
	int sock;		//socket for client
    char name[32];
    double ablaufDatum;
    int sessionId; 
} InternCookie;



typedef struct ExternCookie
{ 
    uint64_t ablaufDatum;
    int sessionId;

} ExternCookie;



InternCookie *createInternCookie(int _socket ,char *name);

int deleteInternCookie(char *name);

InternCookie *FindInternCookie(char *str);


#endif
