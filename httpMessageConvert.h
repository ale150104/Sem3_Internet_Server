#ifndef HTTP_H
#define HTTP_H

#include "network.h"

int strToHTTP(HTTPREQUEST *buffer);

int HTTPTOstr(HTTPREQUEST *buffer, char *strBuf);


#endif