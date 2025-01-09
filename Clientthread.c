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


//Static functions prototype declaration


static void arrayToNull(char *arr, int len);


void *clientthread(void *arg)
{
	InternCookie *self;
	self = NULL;

	int sd = *((int *) arg);

	bool isLoggedIn = false;

	infoPrint("Client thread started.");


	//TODO: Receive messages and send them to all users, skip self

	HTTPREQUEST *httpReq = (HTTPREQUEST *) malloc(sizeof(HTTPREQUEST));
	if(httpReq == NULL){
		errno = ENOMEM;
		errnoPrint("Puffer anlegen für einkommende Nachrichten schief gelaufen");
	} 

	char messageBuffer[MSG_MAX];

	for(;;) {
		int result = networkReceive(sd, messageBuffer);

		if(result > 0) {

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



	if(isLoggedIn == true)
	{
		if(close(self->sock) == -1){
		errorPrint("Socket schließen hat nicht funktioniert!");
		} 
		infoPrint("Socket schließen hat funktioniert");

		if(deleteUser(self->name) == -1){
			errorPrint("User entfernen schief gelaufen");
		} 
		infoPrint("User entfernen hat geklappt!");
	}	
	else 
	{
		if(close(sd) == -1){

		errorPrint("Socket schließen hat nicht funktioniert!");
		} 
		infoPrint("Socket schließen hat funktioniert");
	} 

	free(messageBuffer);

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