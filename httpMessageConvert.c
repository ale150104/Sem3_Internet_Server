#include "httpMessageConvert.h"
#include "network.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


static int getGeneralInfoAboutRequest(HTTPREQUEST *buffer, char *strBuf);

static int getHeadersFromRequest(HTTPREQUEST *buffer, char *strBuf, int maxHeaders);


static char *divideBodyFromHeader(char *str);

static char* buildStatusLine(HTTPRESPONSE *buffer, char *strBuf);

static char* buildHeaderLines(HTTPRESPONSE *buffer, char* strBuf);



int strToHTTP(HTTPREQUEST *buffer, char *strBuf)
{

    //Returncodes
    getGeneralInfoAboutRequest(buffer, strBuf);

    if(strcmp((buffer->Method), "GET") != 0)
    {
        infoPrint("Die Nachricht dürfte einen Body haben");
        //TEMP 
        char *body = divideBodyFromHeader(strBuf);
        infoPrint("Der angegebene HEADER lautet: %s", strBuf);
        
        infoPrint("Der angegebene Body lautet: %s" ,body);
        strcpy(buffer->Body, body);
        buffer->Body[strlen(body)] = 0;  
    } 

    getHeadersFromRequest(buffer, strBuf, 15);

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

    strcpy(httpStr, strBuf); 

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
        //infoPrint("Die %d. Zeile: %s", i, headerlines[i] );

        char *key = strtok(headerlines[i], ":");

        if(key == NULL)
        {
            break;
        } 

        char *value = strtok(NULL, "\r\n");

        //infoPrint("Header-Schlüssel: %s", key);

        //infoPrint("Header-Wert: %s", value);

        if(compareStrings(key, "Content-Length") == true)
        {
            //infoPrint("HEADER Content-Length vorhanden!");
            //Umwandeln in int ...
            buffer->contentLength = atoi(value);
        } 

        else if(compareStrings(key, "Content-Type") == true)
        {
            //infoPrint("HEADER Content-Type vorhanden!");
            strncpy(buffer->contentType, value, 511);
            buffer->contentType[511] = 0; 
        } 

        else if(compareStrings(key, "Host") == true)
        {
            infoPrint("HEADER Host vorhanden!");
            strncpy(buffer->Host, value, 511);
            buffer->Host[511] = 0; 
        } 

        else if(compareStrings(key, "Referer") == true)
        {
            //infoPrint("HEADER Referer vorhanden!");
            strncpy(buffer->Referer, value, 511);
            buffer->Referer[511] = 0; 
        } 

        else if(compareStrings(key, "Connection") == true)
        {
            //infoPrint("HEADER Connection vorhanden!");
            strncpy(buffer->Connection, value, 9);
            buffer->Connection[9] = 0; 
        }
        
        else if(compareStrings(key, "Cookie") == true)
        {
            //infoPrint("HEADER Cookie vorhanden!");
            char *val = strtok(value, "=");

            val = strtok(NULL, ";");

            infoPrint("Mitgesendeter Cookie: %s", val);

            buffer->Cookie = val;
        }  
    }

} 


static char *divideBodyFromHeader(char *str)
{
    
    for(int i = 0; i < strlen(str); i++)
    {

        if(*(str + i) == 13 && *(str + i + 1) == 10 && *(str + i + 2) == 13 && *(str + i + 3) == 10)
        {
            *(str + i + 2) = 0;

            return (str + i +  4);
        } 
    } 

    return NULL;

} 


long int HTTPTOstr(HTTPRESPONSE *buffer, char *strBuf)
{
    cleanUpArray(strBuf, strlen(strBuf));

    char statusLine[512];
    cleanUpArray(statusLine, 512); 

    buildStatusLine(buffer, statusLine);

    infoPrint("Die Status-Line für die HTTP-Response: %s", statusLine);


    char headers[1024];
    cleanUpArray(headers, 1024);

    buildHeaderLines(buffer, headers);

    infoPrint("Die Header: \n%s", headers);

    strcat(strBuf, statusLine);
    strcat(strBuf, headers);

    if(buffer->contentLength > 0)
    {
        strcat(strBuf, "\r\n");

        memmove((strBuf + strlen(strBuf)), buffer->Body, buffer->contentLength);

    } 

    // infoPrint("Die fertige Nachricht:\n%s", strBuf);

    return strlen(headers) + strlen(statusLine) + buffer->contentLength;   
    
    //     char body[MSG_MAX - 512 - 1024];

} 



static char* buildStatusLine(HTTPRESPONSE *buffer, char *strBuf)
{
    strcat(strBuf, buffer->Version);
    
    strcat(strBuf, " ");

    strcat(strBuf, buffer->statusCode);

    strcat(strBuf, " ");

    strcat(strBuf, buffer->statusNachricht);

    strcat(strBuf, "\r\n");

    strcat(strBuf, "\0");

} 


static char* buildHeaderLines(HTTPRESPONSE *buffer, char *strBuf)
{
    //-------------
    strncpy(strBuf, "Content-Length: ", 16);


    char lengthOfContent[7]; 
    sprintf(lengthOfContent, "%d", buffer->contentLength);

    strcat(strBuf, lengthOfContent);

    strcat(strBuf, "\r\n");

    //-----------

    if(buffer->contentLength > 0)
    {
        strcat(strBuf, "Content-Type: ");

        strcat(strBuf, buffer->contentType);

        strcat((strBuf), "\r\n");
    } 


    //---------

    strcat(strBuf, "Connection: ");

    strcat(strBuf, buffer->Connection);

    strcat(strBuf, "\r\n");

    //----------------

        // strcat(strBuf, "Set-Cookie: ");

        // strcat(strBuf, buffer->Cookie);

        // strcat(strBuf, "\r\n");

    // ---------

    strcat(strBuf, "Server: ");

    strcat(strBuf, buffer->Server);

    strcat(strBuf, "\r\n");

    // ----

    strcat(strBuf, "Date: ");

    strcat(strBuf, buffer->Date);

    strcat(strBuf, "\r\n");

    //-----------

    if(strlen(buffer->Cookie) > 0)
    {

        strcat(strBuf, "Set-Cookie: Session=");

        strcat(strBuf, buffer->Cookie);

        strcat(strBuf, "\r\n");
    } 

    strcat(strBuf, "\0");

}

void cleanUpArray(char *strBuf, int length)
{
    for(int i = 0; i < length; i++)
    {
        *(strBuf + i) = 0;
    } 
} 
