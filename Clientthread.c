#define _GNU_SOURCE
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



	//if(isLoggedIn == true)
	//{
	//	if(close(self->sock) == -1){
	//	errorPrint("Socket schließen hat nicht funktioniert!");
	//	} 
	//	infoPrint("Socket schließen hat funktioniert");

		//if(deleteUser(self->name) == -1){
		//	errorPrint("User entfernen schief gelaufen");
		//} 
		//infoPrint("User entfernen hat geklappt!");
	//}	
	//else 
	//{
		if(close(sd) == -1){

		errorPrint("Socket schließen hat nicht funktioniert!");
		} 
		infoPrint("Socket schließen hat funktioniert");
	//} 

	// free(messageBuffer);

	infoPrint("Thread wird nun geschlossen");
	
	pthread_exit(NULL);
	

	debugPrint("Client thread stopping.");
	return NULL;
}



double getTimeInSeconds()
{
	//Timestamp
	struct timespec tv;
	if(clock_gettime(CLOCK_REALTIME, &tv))
		errnoPrint("Fehler beim Zeit auslesen");

	return (double) tv.tv_sec;
} 
