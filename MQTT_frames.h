#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#define PORT 5002
#define staticKeepAlive 0x0A
#define BACKLOG 3

typedef struct{
    int fd;
    char sUsername[100];
    int iKeepAlive;
    int iKeepAliveMax;
    bool bFashion;//0.-None    1.-Subscribe
    bool bFood;
    bool bMusic;
}r_Client;

typedef struct{
    uint8_t bFrameType;
    uint16_t wMsgLen;
    uint16_t wProtlNameLen;
    char sProtName[4];
    uint8_t bVersion;
    uint8_t bConnectFlags;
    uint16_t bKeepAlive;
    uint16_t wClientIdLen;
    char sClientID [6]; //Aqui va el nombre
}f_Connect;

typedef struct{
    uint8_t bFrameType;    
    uint8_t bRemainLen;
    uint8_t bReservedVal;
    uint8_t bReturnCode;
    /*
    0.- Connection accepted, 
    1.- Unaccepted protocol verison
    2.- Identifier rejected
    3.- Server unavailable
    4.- Bad username or password
    5.- Not authorized
    */
}f_ConnAcknowledge;

typedef struct{
    uint8_t bFrameType;
    uint8_t bkeepAlive;
}f_PingRequest;

typedef struct{
    uint8_t bFrameType;
    uint8_t bresponse;
}f_PingResponse;