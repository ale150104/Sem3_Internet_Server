#include <pthread.h>
#include "Cookie.h"
#include "Clientthread.h"
#include <sys/socket.h>
#include <errno.h>
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "network.h"

//Static function prototypes/variables declaration
static bool checkDoubleNamings(const char *name);
static pthread_mutex_t InternCookieLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t sessionLock = PTHREAD_MUTEX_INITIALIZER;

static InternCookie *InternCookieFront = NULL;
static InternCookie *InternCookieBack = NULL;

static int currentAvailableSessionID = 0;


//Cookie anlegen
InternCookie *createInternCookie(int _socket ,char *name){

    pthread_mutex_lock(&InternCookieLock);
    bool isThereAnEntryAlready = checkDoubleNamings(name);
    pthread_mutex_unlock(&InternCookieLock);
    infoPrint("Double Namings checked");

    if(isThereAnEntryAlready == true){

        infoPrint("Ein Eintrag mit dem Namen %s existiert schon", name);

        errno = EEXIST;
        return NULL;
    }

    InternCookie *newEntry = (InternCookie *) malloc(sizeof(InternCookie));
    if(newEntry == NULL){

        errorPrint("Speicher-Allokation beim Anlegen eines InternCookies ist schief gelaufen");

        errno = ENOMEM;
        perror("Speicher-Reservierung ist schief gelaufen!");
        return NULL;
    }
    
    newEntry->prev = NULL;
    newEntry->next = NULL;

    //Werte eintragen in InternCookie struct

    strncpy(newEntry->name, name, strlen(name));
    newEntry->name[strlen(name)] = 0;

    newEntry->ablaufDatum = getTimeInSeconds();

    // newEntry->thread = _thread;
    newEntry->sock = _socket; 

    //Session-ID holen, jedoch synchronisiert, damit zwei Clients nicht dieselbe SessionID bekommen können
    pthread_mutex_lock(&sessionLock);
    newEntry->sessionId = currentAvailableSessionID;
    currentAvailableSessionID++;
    pthread_mutex_unlock(&sessionLock);



    pthread_mutex_lock(&InternCookieLock);

    if(InternCookieFront == NULL && InternCookieBack == NULL){
        //Es ist das erste Element in der Kette
        InternCookieFront = newEntry;
        InternCookieBack = newEntry;
    }

    else{
        InternCookieBack->next = newEntry;
        newEntry->prev = InternCookieBack;
        InternCookieBack = newEntry;
    }

    pthread_mutex_unlock(&InternCookieLock);

    return newEntry;

} 

//Cookie entfernen 
int deleteInternCookie(char *name){
    pthread_mutex_lock(&InternCookieLock);

    InternCookie *Element = InternCookieFront;

    while(Element != NULL){

        if(compareStrings(Element->name, name) == true){


            if(Element->next != NULL && Element->prev != NULL){
                // Mittleres Element

                Element->prev->next = Element->next;
                Element->next->prev = Element->prev;

            } 
            else if(Element->prev == NULL){
                //ERSTES Element der Liste
                InternCookieFront = Element->next;

                //Es gibt ein zweites Element in der Liste
                if(InternCookieFront != NULL){
                    InternCookieFront->prev = NULL;
                }
                //Es gibt kein zweites Element in der Liste
                else{
                    InternCookieBack = NULL;
                } 
            }
            else if(Element->next == NULL){

                //LETZTES Element der Liste
                InternCookieBack = Element->prev;
                InternCookieBack->next = NULL;
            } 

            //TODO: Socket und Thread beenden?
            free(Element);

            pthread_mutex_unlock(&InternCookieLock);
            return 0;
        }

        Element = Element->next;
    }

    pthread_mutex_unlock(&InternCookieLock);

    errno = ENOENT;
    perror("Kein Eintrag gefunden!");
    return -1;
} 


//Über Liste iterieren
InternCookie *FindInternCookie(char *str){
    pthread_mutex_lock(&InternCookieLock);

    InternCookie *Element = InternCookieFront;

    while(Element != NULL){

        InternCookie *nextElement = Element->next;
        
        char strID[10];
        sprintf(strID, "%d", Element->sessionId); 

        infoPrint("Vergleich zwischen: %s und %s", strID, str);
        if(strcmp(strID, str) == 0)
        { 

            pthread_mutex_unlock(&InternCookieLock);

            return Element;

        } 
        Element = nextElement;
    }

    pthread_mutex_unlock(&InternCookieLock);

    return NULL;
} 




static bool checkDoubleNamings(const char *name){
    InternCookie *Element = InternCookieFront;

    while(Element != NULL){
        
        bool result = compareStrings(Element->name, name);
        if(result == true){

            return true;
        }

        Element = Element->next;
    }

    return false;

}


bool compareStrings(const char *str1, const char *str2){

    int str1Length = strlen(str1);

    int str2Length = strlen(str2);

    if(str2Length != str1Length){
        return false;
    }

    for(int i = 0; i < str1Length; i++){
        if((*(str1 + i)) != (*(str2 + i))){
            return false;
        }
    }

    return true;

}
