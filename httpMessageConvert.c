#include "httpMessageConvert.h"
#include "network.h"
#include <string.h>
#include <stdlib.h>


static int getGeneralInfoAboutRequest(HTTPREQUEST *buffer, char *strBuf);

static int getHeadersFromRequest(HTTPREQUEST *buffer, char *strBuf, int maxHeaders);

int strToHTTP(HTTPREQUEST *buffer, char *strBuf)
{
    //Returncodes
    getGeneralInfoAboutRequest(buffer, strBuf);

    getHeadersFromRequest(buffer, strBuf, 7);

    return 0;

} 



static int getGeneralInfoAboutRequest(HTTPREQUEST *buffer, char *strBuf) 
{
    char httpStr[MSG_MAX];

    infoPrint("Array Anelgen hat geklappt");
    strcpy(httpStr, strBuf); 

    infoPrint("Kopieren des Strings hat geklappt");

    // Get Informations about HTTP-Method, Request-Path & HTTP-Version from first Line
    char *firstLine = strtok(httpStr, "\r\n");
    infoPrint("Die erste Zeile der HTTP-Nachricht: \n  %s", firstLine);
    char *httpVerb = strtok(firstLine, " ");

    infoPrint("Das angegebene HTTP-Verb: %s", httpVerb);

    strncpy(buffer->Method, httpVerb, 4);
    buffer->Method[4] = 0; 


    char *url = strtok(NULL, " ");
    infoPrint("Die URL: %s", url);
    strncpy(buffer->Url, url, 511);
    buffer->Url[511] = 0; 

    char *version = strtok(NULL, " ");
    infoPrint("Die Version: %s", version);
    strncpy(buffer->Version, version, 8);
    buffer->Version[8] = 0; 

    return 0;

}  


static int getHeadersFromRequest(HTTPREQUEST *buffer, char *strBuf, int maxHeaders)
{
    char httpStr[MSG_MAX];

    infoPrint("Array Anelgen hat geklappt");
    strcpy(httpStr, strBuf); 

    infoPrint("Kopieren des Strings hat geklappt");

    char *headerlines[maxHeaders];

    strcpy(httpStr, strBuf); 
    char *firstLine = strtok(httpStr, "\r\n");
    infoPrint("Die erste Zeile der HTTP-Nachricht: \n  %s", firstLine);

    for(int i = 0; i < maxHeaders; i++)
    {
        char *headerLine = strtok(NULL, "\r\n");

        headerlines[i] = headerLine;
    }

    for(int i = 0; i < maxHeaders; i++)
    {
        infoPrint("Die %d. Zeile: %s", i, headerlines[i] );

        char *key = strtok(headerlines[i], ":");

        char *value = strtok(NULL, "\r\n");

        infoPrint("Header-Schlüssel: %s", key);

        infoPrint("Header-Wert: %s", value);

        if(compareStrings(key, "Content-Length") == true)
        {
            infoPrint("HEADER Content-Length vorhanden!");
            //Umwandeln in int ...
            buffer->contentLength = atoi(value);
        } 

        if(compareStrings(key, "Content-Type") == true)
        {
            infoPrint("HEADER Content-Type vorhanden!");
            strncpy(buffer->contentType, value, 511);
            buffer->contentType[511] = 0; 
        } 

        if(compareStrings(key, "Host") == true)
        {
            infoPrint("HEADER Host vorhanden!");
            strncpy(buffer->Host, value, 511);
            buffer->Host[511] = 0; 
        } 

        if(compareStrings(key, "Referer") == true)
        {
            infoPrint("HEADER Referer vorhanden!");
            strncpy(buffer->Referer, value, 511);
            buffer->Referer[511] = 0; 
        } 

        if(compareStrings(key, "Connection") == true)
        {
            infoPrint("HEADER Connection vorhanden!");
            strncpy(buffer->Connection, value, 9);
            buffer->Connection[9] = 0; 
        } 
    }

} 




int HTTPTOstr(HTTPREQUEST *buffer, char *strBuf)
{

} 