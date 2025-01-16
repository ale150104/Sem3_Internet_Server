#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <netinet/in.h>
#include <stdbool.h>
#include "Cookie.h"
#include <pthread.h>


void *clientthread(void *arg);


double getTimeInSeconds();

#endif