/*

   nsconn v2.0
   bluetooth connection handler

*/

// bluetooth headers
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "nsconn.h"


// structs containing configuration info defined in main
extern socks_t socks;
extern conf_t runConfig;


void nsConnect(void){


    struct sockaddr_rc addr = { 0 };
    int status;

    errno = 0;
    // create a BT socket using RFCOMM 
    if(socks.insock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)){

        printf("initializing bluetooth socket...\n");

    }else{

        fprintf(stderr, "failed to create socket.\nerror: %s \n", strerror(errno));
        exit(1);
    }
    
    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;

  
    if(str2ba(runConfig.dest, &addr.rc_bdaddr) != 0 ){

        fprintf(stderr, "unable to parse MAC address.\nerror: %s \n", strerror(errno));
        exit(1);

    }

    status = connect(socks.insock, (struct sockaddr *)&addr, sizeof(addr));
    
    // confirm connection has successfully been made
    if(status != 0 ) {

        fprintf(stderr, "failed to connect to headset. check bluetooth is enabled and headset paired.\nerror: %s\n", strerror(errno));
        exit(1);

    } else { 

        printf("connected to headset with MAC %s \n", runConfig.dest); 

    }












}