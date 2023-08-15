
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "nsconn.h"


// parsing packets

#define SYNC          0xAA
#define EXCODE        0x55

extern socks_t socks;
extern conf_t runConfig;


void parseFail(){
    printf("unable to read headset data.\n");
    exit(1);
}

void parsePackets(void){

    int failCount = 0;    // check for consistent failure

    while(1){
        
        // see if this works 
        if((runConfig.netOut == true) && (socks.clientsock < 1)){
            printf("Client disconnected.\n");
            break;
            close(socks.outsock);
            close(socks.insock);
        }
        if(failCount == 10) {
            parseFail();
        }

        unsigned int bufferIn[1] = {0};
        int pLength;
        unsigned char payload[256];
        

        // read a byte
        recv(socks.insock,bufferIn,1,0);

        // check for sync byte
        if( *bufferIn != SYNC ){
            printf("sync fail.\n");  
            failCount += 1;  
            continue; // restart the process
        }

        // check sync byte 2 if byte 1 ok
        recv(socks.insock, bufferIn, 1, 0);

        if( *bufferIn != SYNC ){
            printf("sync fail 2.\n"); // error on fail (AA byte 2)
            continue;
        } else {  // if synchronisation is successful
            recv(socks.insock,bufferIn,1,0);

            pLength = *bufferIn;

            if( pLength >= 170 ) {
                printf("payload greater that 170\n");
                break;
            }
          
            for(int pb = 0; pb < pLength; pb++){
                recv(socks.insock,bufferIn,1,0); // next byte is payload length
                payload[pb] = *bufferIn;
            }

            //  *buffer = 0;  // clear buffer
            recv(socks.insock,bufferIn,1,0);
            int checksum = *bufferIn;

            // calculate checksum for comparison with provided payload
            int thischecksum = 0;
            for(int i=0; i<pLength; i++) thischecksum += payload[i];

            // flip the bits (1's compliment) 
            thischecksum &= 0xFF;
            thischecksum = ~thischecksum & 0xFF;

            // compare calculated checksum to checksum byte
            if(checksum != thischecksum) {
                printf("Payload != checksum\n");
                continue; // terminate and parse a new packet
            }

            // send for further processing if JSON in use
            if(runConfig.format == JSON){
                jsonFormat(payload, pLength);
            } else if (runConfig.format == BINARY){    // forward raw binary
                // print to screen if stdout enabled
                if (runConfig.output = true){
                    for (int i = 0; i < pLength; i++){
                        printf("%02X", payload[i]);
                    }
                    printf("\n"); // create new line between each object
                }
                
                forward((const char *)payload);

            }
        }
    }
}
