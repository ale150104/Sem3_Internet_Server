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


//Static functions prototype declaration


static void arrayToNull(char *arr, int len);

static int sendBadRequest();

static int sendServerError();

static int sendNotFound();

static void getTimeInPretty(char *_time);


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
					infoPrint("Die Pseudo-PDF wurde angefragt");
					//...
				}

				if(compareStrings(httpReq->Url, "/mixed") == true)
				{
					infoPrint("Die Pseudo-TEXT+BILD-Datei wurde angefragt");
					//...
				}

				if(compareStrings(httpReq->Url, "/myname") == true)
				{
					infoPrint("Der aktuelle Name wurde angefragt");
					//...
				}

				else{
					sendBadRequest(sd);
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
			}   

		}

		else if(result == 0) {

			infoPrint("Client hat Verbindung geschlossen");


			break;
		}
		else {
			
			errnoPrint("Beim Lesen einer Nachricht ist etwas schief gelaufen");
			break;

		}

	}


	if(close(sd) == -1){

	errorPrint("Socket schließen hat nicht funktioniert!");
	} 
	infoPrint("Socket schließen hat funktioniert");


	infoPrint("Thread wird nun geschlossen");
	
	pthread_exit(NULL);
	

	debugPrint("Client thread stopping.");
	return NULL;
}


static int sendServerError()
{

} 

static int sendBadRequest(int socket)
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

	strncpy(resp->statusCode,"400", 4);
	resp->statusCode[3] = 0;

	infoPrint("Der Code: %s", resp->statusCode);

	strncpy(resp->statusNachricht, "BadRequest", 11);
	resp->statusNachricht[11] = 0;

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

static void getTimeInPretty(char *_time)
{
	time_t t;

    /* Retrieve the current time */
    t = time(NULL);

	size_t writtenChars = strftime(_time, 100, "%a, %d %b %Y %X %Z", localtime(&t));

	*(_time + writtenChars + 1) = 0;

	infoPrint("Aktuelle Zeit auf dem Server: %s", _time);

} 


static int sendNotFound()
{

} 
 
