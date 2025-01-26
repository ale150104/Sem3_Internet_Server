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

static int sendErrorResponse(int socket, char *statusCode, char *statusText);

static int sendSuccessResponse(int socket, long int sizeOfBody, char *contentType, char *body, InternCookie *cookie);

static int validateRequest(HTTPREQUEST *httpReq);

static int copyFile(char *dest, char *fileName, long int bufferSize);

static long int createMixedResponse(char *strBuf, long int sizeOfStrBuf, char *text, char *boundary);


void *clientthread(void *arg)
{
	InternCookie *self;
	self = NULL;

	bool wasCookieInHeader;
	
	wasCookieInHeader = false;

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


			//Validate Cookie
			if(httpReq->Cookie != NULL)
			{
				infoPrint("Ein Cookie wurde mitgeschickt");
				 self = FindInternCookie(httpReq->Cookie);


				if(self == NULL)
				{
					errnoPrint("Cookie nicht gefunden");
					sendErrorResponse(sd, "400", "BadRequest");
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
					sendErrorResponse(sd, "500", "Server Error");
					break;
				}  

			} 	

			//See Which Resource was requested
			if(compareStrings(httpReq->Method, "GET") == true)
			{
				infoPrint("Eine Resource wurde mit der GET-Methode angefragt");
				if(compareStrings(httpReq->Url, "/website") == true)
				{
					long int sizeOfFile = findSize("assets/rwu.html");
					if(sizeOfFile <= 0)
					{
						sendErrorResponse(sd, "500", "ServerError");
						break;
					} 

					infoPrint("Size of file: %ld", sizeOfFile);

					char body[sizeOfFile]; 

					 int res = copyFile(body, "assets/rwu.html", sizeOfFile);

					 body[sizeOfFile] = 0; 

					 if(res < 0)
					 {
						sendErrorResponse(sd, "500", "ServerError");
					 } 

					// infoPrint("in Body steht: %s", body);
					
					sendSuccessResponse(sd, sizeOfFile, "text/html", body, self);
					break;
				}
				
				else if(compareStrings(httpReq->Url, "/pdf") == true)
				{
					long int sizeOfFile = findSize("assets/spo.pdf");
					if(sizeOfFile <= 0)
					{
						sendErrorResponse(sd, "500", "ServerError");
						break;
					} 

					infoPrint("Size of file: %ld", sizeOfFile);

					char body[sizeOfFile]; 

					 int res = copyFile(body, "assets/spo.pdf", sizeOfFile);

					 if(res < 0)
					 {
						sendErrorResponse(sd, "500", "ServerError");
					 } 

					infoPrint("in Body steht: %s", body);
					
					sendSuccessResponse(sd, sizeOfFile, "application/pdf", body, self);
					break;


				}

				else if(compareStrings(httpReq->Url, "/mixed") == true)
				{
					infoPrint("Die Pseudo-TEXT+BILD-Datei wurde angefragt");
					
					char body[2095000];
	
 

					long int size = createMixedResponse(body, 2095000, "Das ist das wunderschöne H-Gebäude", "--------------------------104850386028541947603269");

					sendErrorResponse("sd", "400", "Bad");
				}

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

			else if(compareStrings(httpReq->Method, "POST") == true) 
			{
				infoPrint("Eine Ressource wurde mit POST-Methode angefragt");
				
				if(compareStrings(httpReq->Url, "/myname") == true)
				{
					if(wasCookieInHeader == false)
					{
						errnoPrint("Ohne Aktiven Cookie kein POST");
						deleteInternCookie(self->name);
						sendErrorResponse(sd, "400", "BadRequest");
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

	cleanUpArray(resp->Cookie, 512);

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

		memmove(resp->Body, body, sizeOfBody);
		//resp->Body = body;
	} 

	// TODO: Set-Cookie...
	if(cookie != NULL)
	{
		char sessionId[10];
		sprintf(sessionId, "%d", cookie->sessionId);

		infoPrint("Die Session für den Cookie: %s", sessionId);

		strncpy(resp->Cookie,sessionId, strlen(sessionId));
		resp->Cookie[strlen(sessionId)] = 0;

	} 


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

	long int sizeOfStr = HTTPTOstr(resp, respStr);

	//Send
	//...

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

	infoPrint("Größe der Datei: %ld", res);
  
    // closing the file 
    fclose(fp); 
  
    return res; 
} 


// -1 for error
// 0 success
static int copyFile(char *dest, char *fileName, long int bufferSize)
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

	size_t read = fread(dest, 1, bufferSize, fptr);
	infoPrint("Gelesene Bytes aus Datei: %ld", read);

	if(bufferSize != read)
	{
		return -1;
	} 

	//infoPrint("BUffer enhält: %s", );

	fclose(fptr);
	
	
	infoPrint("Größe des Bodys in Byte: %ld\n ", bufferSize);




	return 0;

} 




static long int createMixedResponse(char *strBuf, long int sizeOfStrBuf, char *text, char *boundary)
{

	strcat(strBuf, boundary);
	strcat(strBuf, "\r\n");
	strcat(strBuf, 'Content-Disposition: form-data; name="Description_Text"\r\n');

	strcat(strBuf, text);

	strcat(strBuf, boundary);
	strcat(strBuf, "\r\n");

	char temp1[100];
	strcpy(temp1, (char *)'Content-Disposition: form-data; name= "Picture1"; filename="pic.webp"\r\n'); 
	strcat(strBuf, temp1);

	strcat(strBuf, "Content-Type: image/webp");


	//...
	long int sizeOfFile = findSize("assets/H_Gebaeude.webp");
	if(sizeOfFile <= 0)
	{
		return -1;
	} 

	infoPrint("Size of file: %ld", sizeOfFile);

	char img[sizeOfFile]; 

	int res = copyFile(img, "assets/H_Gebaeude.webp", sizeOfFile);

	if(res < sizeOfFile)
	{
		return -1;
	} 

	memmove((strBuf + strlen(strBuf)), img, sizeOfFile);



	strcat(strBuf, boundary);

	infoPrint("Die berechnete Länge von diesem Body ist: %ld", strlen(strBuf));

	infoPrint("Der Body: %s", strBuf);

} 