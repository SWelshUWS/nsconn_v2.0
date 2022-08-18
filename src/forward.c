#include "nsconn.h"

extern conf_t runConfig;
extern socks_t socks;

// decide what to do with the parsed data
int forward(const char * stringToSend){

    int stlen;
    // get length of final object
    for (stlen = 0; stringToSend[stlen] != '\0'; ++stlen);
  

    errno = 0;
    // if networking enabled, send to socket
    if(runConfig.netOut == true) {

        if (send(socks.clientsock, stringToSend, stlen, 0) == -1){
            fprintf(stderr, "could not send object to client %s\n", strerror(errno));         

        }
            
    }     
    // if stdout enabled
    if(runConfig.output == true) { 

        if(runConfig.format == JSON){

            // in future should have option to allow commas between objects
            printf("%s\n", stringToSend); 

        } 
    }
      
}


