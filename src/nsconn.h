#ifndef NSCONN_H
#define NSCONN_H


// global header includes
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/socket.h> // access to read/send functions


// network types
#define NETSOCK     int
#define BTSOCK      int
#define PORT        char *
#define MACADDR     char *

// output format values
#define BINARY      0
#define JSON        1


// struct to hold sockets/connections
typedef struct {

    BTSOCK      insock;
    NETSOCK     outsock;        // initialize to -1 if not in use
    NETSOCK     clientsock;
    PORT        port;           // ditto

} socks_t;


// struct to hold configuration details
typedef struct {

    PORT        portOut;   // port to use
    MACADDR     dest;      // destination MAC for Bluetooth connection
    int         output;    // toggle print payload to stdout
    int         netOut;    // toggle networking
    int         rawwavOff; // toggle disable raw eeg wave output
    int         format;    // define output format JSON/binary
    
} conf_t;


// function declarations

// btsock.c
void nsConnect(void);

// start outbound socket
void sockStart(void);

// outbound socket listen
void listenForClients(void);

// parse packets
void parsePackets(void);

// json format
int jsonFormat(unsigned char *, int sz);

// forward json strings
int forward(const char *);


#endif