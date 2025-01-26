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

static int sendErrorResponse(int socket, char *statusCode, char *statusText);

static int sendSuccessResponse(int socket, long int sizeOfBody, char *contentType, char *body, InternCookie *cookie);

static int validateRequest(HTTPREQUEST *httpReq);

static long int createMixedResponse(char *strBuf, long int sizeOfStrBuf, char *text, char *boundary);


void *clientthread(void *arg)
{
	//Intern struct for managing Cookie-Session
	InternCookie *self;
	self = NULL;

	//Important for Checking if Client wants to change his name eventhough he haven't had a Cookie when requesting
	bool wasCookieInHeader;
	
	wasCookieInHeader = false;

	// Socket Descriptor
	int sd = *((int *) arg);

	// bool isLoggedIn = false;

	infoPrint("Client thread started.");

	// Internal struct for having Informations about Request in a clean struct
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

			// Converting the HTTP-Request (Just a stream of Characters into the struct)
			if (strToHTTP(httpReq, messageBuffer) != 0)
			{
				errorPrint("Fehler aufgetreten beim Bauen der HTTP-Request intern");
			}

			// Validate that the right Host is requested and that Client knows about Server just offering Connection-Type 'close'
			int validation = validateRequest(httpReq); 

			if(validation != 0)
			{
				sendErrorResponse(sd, "400", "Bad Request");
				break;
			}


			//Validate Cookie
			// If an non existing Cookie is sent by Client, Server sends Error back to Client
			if(httpReq->Cookie != NULL)
			{
				infoPrint("Ein Cookie wurde mitgeschickt");
				 self = FindInternCookie(httpReq->Cookie);


				if(self == NULL)
				{
					errnoPrint("Cookie nicht gefunden");
					sendErrorResponse(sd, "400", "Bad Request");
					break;
				} 

				wasCookieInHeader = true;

			} 
			else 
			{
				errorPrint("Kein Cookie wurde mitgeschickt");
				self = createInternCookie(sd, "Unknown");
				infoPrint("Angelegter Cookie mit Name: %s", self->name);
				if(self == NULL)
				{
					errnoPrint("Beim Anlegen des Cookies ist etwas schief gelaufen");
					sendErrorResponse(sd, "500", "Internal Server Error");
					break;
				}  

			} 	

			//See Which Resource was requested
			
			if(compareStrings(httpReq->Method, "GET") == true)
			{
				infoPrint("Eine Resource wurde mit der GET-Methode angefragt");

				//Website was requested
				//Internal Server Error when having I/O issues, else sending Success Response with Payload
				if(compareStrings(httpReq->Url, "/website") == true)
				{
					long int sizeOfFile = findSize("assets/rwu.html");
					if(sizeOfFile <= 0)
					{
						sendErrorResponse(sd, "500", "Internal Server Error");
						break;
					} 

					infoPrint("Size of file: %ld", sizeOfFile);

					char body[sizeOfFile]; 

					 int res = copyFile(body, "assets/rwu.html", sizeOfFile);

					 body[sizeOfFile] = 0; 

					 if(res < 0)
					 {
						sendErrorResponse(sd, "500", "Internal Server Error");
					 } 
					
					sendSuccessResponse(sd, sizeOfFile, "text/html", body, self);
					break;
				}
				
				//PDF was requested
				//Internal Server Error when having I/O issues, else sending Success Response with Payload
				else if(compareStrings(httpReq->Url, "/pdf") == true)
				{
					long int sizeOfFile = findSize("assets/spo.pdf");
					if(sizeOfFile <= 0)
					{
						sendErrorResponse(sd, "500", "Internal Server Error");
						break;
					} 

					infoPrint("Size of file: %ld", sizeOfFile);

					char body[sizeOfFile]; 

					 int res = copyFile(body, "assets/spo.pdf", sizeOfFile);

					 if(res < 0)
					 {
						sendErrorResponse(sd, "500", "Internal Server Error");
					 } 

					infoPrint("in Body steht: %s", body);
					
					sendSuccessResponse(sd, sizeOfFile, "application/pdf", body, self);
					break;


				}

				//Multipart was requested
				//Internal Server Error when having I/O issues or during construction of body, else sending Success Response with Payload
				else if(compareStrings(httpReq->Url, "/mixed") == true)
				{
					infoPrint("Die Pseudo-TEXT+BILD-Datei wurde angefragt");
					
					char body[2095000];

					long int size = createMixedResponse(body, 2095000, "Das ist das wunderschöne H-Gebäude", "--------------------------104850386028541947603269");

					if(size < 0)
					{
						sendErrorResponse(sd, "500", "Internal Server Error");
						break;
					} 

					sendSuccessResponse(sd, size, "multipart/form-data; boundary=--------------------------104850386028541947603269", body, self);
					break;
				}

				//Username for Cookie-Session was requested
				//Internal Server Error when having I/O issues, else sending Success Response with Payload
				else if(compareStrings(httpReq->Url, "/myname") == true)
				{

					char body[100];
					strncpy(body, "Name=", 6);
					body[5] = 0;

					strcat(body, self->name);

					body[strlen(body)] = 0; 

					infoPrint("Name of requesting client: %s", self->name);

					infoPrint("Body: %s", body);
					int bodyLength = strlen(body);
					sendSuccessResponse(sd, bodyLength, "text/plain", body, self);
					break;
				}

				else{
					sendErrorResponse(sd, "404", "Not Found");
					break;
				} 
			}

			//Changing Name was requested
			//Internal Server Error when having Permission issues, else sending Success Response with New name
			else if(compareStrings(httpReq->Method, "POST") == true) 
			{
				infoPrint("Eine Ressource wurde mit POST-Methode angefragt");
				
				if(compareStrings(httpReq->Url, "/myname") == true)
				{
					if(wasCookieInHeader == false)
					{
						errnoPrint("Ohne Aktiven Cookie kein POST");
						deleteInternCookie(self->name);
						sendErrorResponse(sd, "400", "Bad Request");
						break;
					} 

					infoPrint("Die Namensänderung wurde angefragt");

					char *name = strtok(httpReq->Body, "=");
					name = strtok(NULL, ";");
					infoPrint("Der Neue Name des Clients soll sein: %s", name);

					strncpy(self->name, name, strlen(name));
					self->name[strlen(name)] = 0;

					
					char body[100];
					strncpy(body, "Name=", 6);
					body[5] = 0;

					strcat(body, self->name);

					body[strlen(body)] = 0; 

					infoPrint("Name of requesting client: %s", self->name);

					infoPrint("Body: %s", body);
					int bodyLength = strlen(body);
					sendSuccessResponse(sd, bodyLength, "text/plain", body, self);
					break;
				
				}
				else
				{
					sendErrorResponse(sd, "404", "Not Found");
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
			sendErrorResponse(sd, "500", "Internal Server Error");
			break;

		}

	}


	// Cleanup when Closing Connection to Client
	if(close(sd) == -1){

	errorPrint("Socket schließen hat nicht funktioniert!");
	} 
	infoPrint("Socket schließen hat funktioniert");


	infoPrint("Thread wird nun geschlossen");
	
	pthread_exit(NULL);
}



// HTTP-Error-Response to Client
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

	//The StatusCode for the Request which ended up with issues
	strncpy(resp->statusCode,statusCode, 4);
	resp->statusCode[3] = 0;

	infoPrint("Der Code: %s", resp->statusCode);

	// The Corresponding Status Message for the StatusCode
	strncpy(resp->statusNachricht, statusText, strlen(statusText));
	resp->statusNachricht[strlen(statusText)] = 0;

	infoPrint("Die Nachricht: %s", resp->statusNachricht);

	// No Body being sent to Client when having an Error
	resp->contentLength = 0;

	// Signaling Client that Connection is going to be closed after the Server did send the response
	strncpy(resp->Connection,"close", 5);
	resp->Connection[5] = 0;

	cleanUpArray(resp->Cookie, 512);

	//DATE-Field in Header for the Response
	char time[100];
	getTimeInPretty(time);
	strncpy(resp->Date, time, strlen(time));
	resp->Date[strlen(time)] = 0; 


	//Server-Field for the Response
	strncpy(resp->Server,"LAyerServer", 11);
	resp->Server[11] = 0;

	char respStr[MSG_MAX]; 

	//Converting internal Response-Struct to appropriate HTTP-Message for Sending to Client
	HTTPTOstr(resp, respStr);

	networkSend(socket, respStr, strlen(respStr));

} 




static int sendSuccessResponse(int socket, long int sizeOfBody, char *contentType, char *body, InternCookie *cookie)
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

	//Status Code for showing to Client that all went Ok
	strncpy(resp->statusCode, "200", 4);
	resp->statusCode[3] = 0;

	infoPrint("Der Code: %s", resp->statusCode);

	// The Corresponding Status Message for the StatusCode
	strncpy(resp->statusNachricht, "Ok", 3);
	resp->statusNachricht[2] = 0;

	infoPrint("Die Nachricht: %s", resp->statusNachricht);

	//Length of the Body being sent to the Client with Payload (If Body present)
	resp->contentLength = sizeOfBody;

	// Adding Information about the Content-Type of the Body, if there is one
	if(sizeOfBody > 0)
	{
		strncpy(resp->contentType, contentType, strlen(contentType));
		resp->contentType[strlen(contentType)] = 0;

		memmove(resp->Body, body, sizeOfBody);
	} 

	// Information about the Cookie for the Client
	if(cookie != NULL)
	{
		char sessionId[10];

		sprintf(sessionId, "%d", cookie->sessionId);

		infoPrint("Die Session für den Cookie: %s", sessionId);

		strncpy(resp->Cookie,sessionId, strlen(sessionId));
		resp->Cookie[strlen(sessionId)] = 0;

	} 


	// Signaling Client that Connection is going to be closed after the Server did send the response
	strncpy(resp->Connection,"close", 5);
	resp->Connection[5] = 0;

	
	//DATE-Field in Header for the Response
	char time[100];
	getTimeInPretty(time);
	strncpy(resp->Date, time, strlen(time));
	resp->Date[strlen(time)] = 0; 
	infoPrint("Länge des Datums: %ld", strlen(time));



	//Server-Field for the Response
	strncpy(resp->Server,"LAyerServer", 11);
	resp->Server[11] = 0;

	char respStr[MSG_MAX]; 

	long int sizeOfStr = HTTPTOstr(resp, respStr);

	networkSend(socket, respStr, sizeOfStr);

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
 

static long int createMixedResponse(char *strBuf, long int sizeOfStrBuf, char *text, char *boundary)
{

	cleanUpArray(strBuf, sizeOfStrBuf);

	long int sizeOfBody;
	sizeOfBody = 0;

	infoPrint("Wert von SizeOfBody: %ld", sizeOfBody);

	// Starting Boundary
	strcat(strBuf, boundary);
	sizeOfBody+= strlen(boundary);

	strcat(strBuf, "\r\n");
	sizeOfBody += 2;

	strcat(strBuf, "Content-Disposition: form-data; name=""Description_Text""\r\n");
	sizeOfBody += 59;

	strcat(strBuf, text);
	sizeOfBody += strlen(text);

	strcat(strBuf, "\r\n");
	sizeOfBody += 2;

	// Splitting Boundary from first Content Block
	strcat(strBuf, boundary);
	sizeOfBody+= strlen(boundary);

	strcat(strBuf, "\r\n");
	sizeOfBody += 2;

	strcat(strBuf, "Content-Disposition: form-data; name= ""Picture1""; filename=""pic.webp""\r\n"); 
	sizeOfBody +=75;

	strcat(strBuf, "Content-Type: image/webp; charset=utf-8\r\n\r\n");
	sizeOfBody += 43;


	//...
	long int sizeOfFile = findSize("assets/H_Gebaeude.webp");
	if(sizeOfFile <= 0)
	{
		return -1;
	} 

	char img[sizeOfFile]; 

	int res = copyFile(img, "assets/H_Gebaeude.webp", sizeOfFile);

	if(res < 0)
	{
		return -1;
	} 

	sizeOfBody += sizeOfFile;

	memmove((strBuf + strlen(strBuf)), img, sizeOfFile);

	strcat(strBuf, "\r\n");
	sizeOfBody += 2;
	
	// Splitting Boundary from second and last Content Block
	strcat(strBuf, boundary);
	sizeOfBody+= strlen(boundary);

	infoPrint("Die berechnete Länge von diesem Body ist: %ld", sizeOfBody);

	infoPrint("Der Body: %s", strBuf);

	return sizeOfBody;

} 