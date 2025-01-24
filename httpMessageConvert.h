#ifndef HTTP_H
#define HTTP_H

#include "network.h"
#include "util.h"
#include "Cookie.h"

int strToHTTP(HTTPREQUEST *buffer, char *strBuf);

long int HTTPTOstr(HTTPRESPONSE *buffer, char *strBuf);


#endif