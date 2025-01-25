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




bool compareStrings(const char *str1, const char *str2);
//TODO: Add prototypes for functions that fulfill the following tasks:
// * Add a new user to the list and start client thread
// * Iterate over the complete list (to send messages to all users)
// * Remove a user from the list
//CAUTION: You will need proper locking!

#endif
