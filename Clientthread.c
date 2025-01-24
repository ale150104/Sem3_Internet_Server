#include "Clientthread.h"
#include "Cookie.h"
#include "util.h"
#include "network.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "httpMessageConvert.h"
#include <stdio.h>


//Static functions prototype declaration

static long int findSize(char file_name[]);

static void arrayToNull(char *arr, int len);

static int sendErrorResponse(int socket, char *statusCode, char *statusText);

static int sendSuccessResponse(int socket, int sizeOfBody, char *contentType, char *body);

static int validateRequest(HTTPREQUEST *httpReq);

static int copyFile(char *dest, char *fileName);


void *clientthread(void *arg)
{
	InternCookie *self;
	self = NULL;

	int sd = *((int *) arg);

	// bool isLoggedIn = false;

	infoPrint("Client thread started.");

	HTTPREQUEST *httpReq = (HTTPREQUEST *) malloc(sizeof(HTTPREQUEST));
	if(httpReq == NULL){
		errno = ENOMEM;
		errnoPrint("Puffer anlegen für einkommende Nachrichten schief gelaufen");
	} 

	char messageBuffer[MSG_MAX];

	for(;;) {
		int result = networkReceive(sd, messageBuffer);

		if(result > 0) {
			infoPrint("Ein HTTP-Request ist angekommen");

			infoPrint("Raw HTTP-Nachricht:");
			infoPrint("%s", messageBuffer);

			if (strToHTTP(httpReq, messageBuffer) != 0)
			{
				errorPrint("Fehler aufgetreten beim Bauen der HTTP-Request intern");
			}

			int validation = validateRequest(httpReq); 

			if(validation != 0)
			{
				sendErrorResponse(sd, "400", "BadRequest");
				break;
			} 

			//See Which Resource was requested
			if(compareStrings(httpReq->Method, "GET") == true)
			{
				infoPrint("Eine Resource wurde mit der GET-Methode angefragt");
				if(compareStrings(httpReq->Url, "/website") == true)
				{
					infoPrint("Die Pseudo-Webseite wurde angefragt");
					//...
				}
				
				else if(compareStrings(httpReq->Url, "/pdf") == true)
				{
					long int sizeOfFile = findSize("assets/spo.pdf");
					if(sizeOfFile <= 0)
					{
						sendErrorResponse(sd, "500", "ServerError");
						break;
					} 

					char body[sizeOfFile]; 

					int res = copyFile(body, "assets/spo.pdf");

					if(res < 0)
					{
						sendErrorResponse(sd, "500", "ServerError");
						break;
					} 

					infoPrint("in Body steht: %s", body);

					//send valid response
					//...
					sendSuccessResponse(sd, sizeOfFile, "application/pdf", body);
					break;


				}

				else if(compareStrings(httpReq->Url, "/mixed") == true)
				{
					infoPrint("Die Pseudo-TEXT+BILD-Datei wurde angefragt");
					//...
				}

				else if(compareStrings(httpReq->Url, "/myname") == true)
				{
					infoPrint("Der aktuelle Name wurde angefragt");
					//...
				}

				else{
					sendErrorResponse(sd, "404", "Not found");
					break;
				} 
			}

			else if(compareStrings(httpReq->Method, "POST") == true) 
			{
				infoPrint("Eine Ressource wurde mit POST-Methode angefragt");
				
				if(compareStrings(httpReq->Url, "/myname") == true)
				{
					infoPrint("Die Namensänderung wurde angefragt");
					//...
				}
				else
				{
					sendErrorResponse(sd, "404", "Not found");
					break;
				} 
			}   

		}

		else if(result == 0) {

			infoPrint("Client hat Verbindung geschlossen");


			break;
		}
		else {
			
			errnoPrint("Beim Lesen einer Nachricht ist etwas schief gelaufen");
			sendErrorResponse(sd, "500", "ServerError");
			break;

		}

	}


	if(close(sd) == -1){

	errorPrint("Socket schließen hat nicht funktioniert!");
	} 
	infoPrint("Socket schließen hat funktioniert");


	infoPrint("Thread wird nun geschlossen");
	
	pthread_exit(NULL);
}



static int sendErrorResponse(int socket, char *statusCode, char *statusText)
{
	HTTPRESPONSE *resp = (HTTPRESPONSE *) malloc(sizeof(HTTPRESPONSE));
	if(resp == NULL)
	{
		errnoPrint("Beim Anlegen von HTTPRESPONSE-Speicher ist etwas schiefgelaufen");
		return -1;
	} 


	strncpy(resp->Version,"HTTP/1.1", 9);
	resp->Version[8] = 0;

	infoPrint("Die Version: %s", resp->Version);

	strncpy(resp->statusCode,statusCode, 4);
	resp->statusCode[3] = 0;

	infoPrint("Der Code: %s", resp->statusCode);

	strncpy(resp->statusNachricht, statusText, strlen(statusText));
	resp->statusNachricht[10] = 0;

	infoPrint("Die Nachricht: %s", resp->statusNachricht);

	resp->contentLength = 0;

	strncpy(resp->Connection,"close", 5);
	resp->Connection[5] = 0;

	//DATE
	//...
	char time[100];
	getTimeInPretty(time);
	strncpy(resp->Date, time, strlen(time));
	resp->Date[strlen(time)] = 0; 
	infoPrint("Länge des Datums: %ld", strlen(time));


	strncpy(resp->Server,"LAyerServer", 11);
	resp->Server[11] = 0;

	//convertToStr
	//...
	char respStr[MSG_MAX]; 

	HTTPTOstr(resp, respStr);

	//Send
	//...

	networkSend(socket, respStr);

} 




static int sendSuccessResponse(int socket, int sizeOfBody, char *contentType, char *body)
{
	HTTPRESPONSE *resp = (HTTPRESPONSE *) malloc(sizeof(HTTPRESPONSE));
	if(resp == NULL)
	{
		errnoPrint("Beim Anlegen von HTTPRESPONSE-Speicher ist etwas schiefgelaufen");
		return -1;
	} 


	strncpy(resp->Version,"HTTP/1.1", 9);
	resp->Version[8] = 0;

	infoPrint("Die Version: %s", resp->Version);

	strncpy(resp->statusCode, "200", 4);
	resp->statusCode[3] = 0;

	infoPrint("Der Code: %s", resp->statusCode);

	strncpy(resp->statusNachricht, "Ok", 3);
	resp->statusNachricht[2] = 0;

	infoPrint("Die Nachricht: %s", resp->statusNachricht);

	resp->contentLength = sizeOfBody;

	if(sizeOfBody > 0)
	{
		strncpy(resp->contentType, contentType, strlen(contentType));
		resp->contentType[strlen(contentType)] = 0;

		resp->Body = body;
	} 

	// TODO: Set-Cookie...

	strncpy(resp->Connection,"close", 5);
	resp->Connection[5] = 0;

	//DATE
	//...
	char time[100];
	getTimeInPretty(time);
	strncpy(resp->Date, time, strlen(time));
	resp->Date[strlen(time)] = 0; 
	infoPrint("Länge des Datums: %ld", strlen(time));


	strncpy(resp->Server,"LAyerServer", 11);
	resp->Server[11] = 0;

	//convertToStr
	//...
	char respStr[MSG_MAX]; 

	HTTPTOstr(resp, respStr);

	//Send
	//...

	networkSend(socket, respStr);

} 



static int validateRequest(HTTPREQUEST *httpReq)
{
	if(strcmp(httpReq->Host, " LAyerServer") != 0)
	{
		infoPrint("Wert von Host: %s", httpReq->Host);
		errnoPrint("Der Host ist falsch!");
		return -1;
	}


	if(strcmp(httpReq->Connection, " close") != 0)
	{
		errnoPrint("Der Server bietet nur Verbindungen an, die nach Antwort geschlossen werden");
		return -1;
	} 

	infoPrint("Request-Validierung war erfolgreich");

	return 0; 
} 
 
 static long int findSize(char file_name[]) 
{ 
    // opening the file in read mode 
    FILE* fp = fopen(file_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    long int res = ftell(fp); 
  
    // closing the file 
    fclose(fp); 
  
    return res; 
} 


// -1 for error
// 0 success
static int copyFile(char *dest, char *fileName)
{
	FILE* fptr;
	if ((fptr = fopen(fileName, "rb")) == NULL) 
	{
		printf("Error! opening file");
			
		// If file pointer will return NULL
		// Program will exit.
		return -1;
	}
		
	// else it will return a pointer 
	// to the file.

	size_t read = fread(dest, sizeof(dest), 1, fptr);
	infoPrint("Gelesene Bytes aus Datei: %ld", read);

	infoPrint("BUffer enhält: %s", dest);

	fclose(fptr);
	infoPrint("Die Pseudo-PDF wurde angefragt");
	infoPrint("Größe des Bodys in Byte: %ld", sizeof(dest));

	return 0;

} 
