#include <errno.h>
#include "network.h"
#include <unistd.h>
#include <stdlib.h>
#include "util.h"
#include <arpa/inet.h>
#include <string.h>
#include "Cookie.h"

//Return-value 1 : Recieving Message was ok
//Return-value 0: Client closed Connection
//Return-value -1: Error while reading Message

int networkReceive(int fd, char *buffer)
{
	//TODO: Receive length
	ssize_t bytes_read;

	bytes_read = read(fd, buffer, MSG_MAX);

	if(bytes_read > 0) {
		return 1;
	}
	else if (bytes_read == 0) {
		return 0;

	}

	return -1;
}


int networkSend(int fd, const char *buffer)
{

	// uint16_t messageLength = ntohs(buffer->length);

	// infoPrint("Länge der zu versendenden Nachricht: %d, Nachricht-Payload: %s", messageLength, buffer->body.serverToClient.Text);


	// ssize_t writtenBytes = write(fd, buffer, 3 + messageLength);
	
	// infoPrint("Geschrieben Bytes: %ld", writtenBytes);

	// if(writtenBytes == -1){
	//	errno = ENOSYS;
	//	return -1;
	// }

	return 0;

}
