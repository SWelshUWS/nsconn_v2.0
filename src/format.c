/*
    nsconn v2.0
    Stacey Welsh (June 2022)

    contains the json formatter, sends output to forward()

    note: 
    a time field is not included in the original ThinkGear connector 
    but I though it would probably be useful to get a timestamp of
    exactly when the value comes in, especially given that other 
    scripting languages can be slower in comparison to C (such as
    Python  https://www.github.com/SWelshsUWS/nsconn-influxdb.py
    with InfluxDB for an example of this).This isn't so important when
    the raw wave value has been turned off.

    Although the excode value 0x55 has been defined in the original 
    ThinkGear Communications protocol guide, it has been omitted in 
    this implementation as although NeuroSky have stated that they 
    may implement this in the future, it is not used in any of their
    headsets currently and would therefore just be a waste of code.
    See:
    http://developer.neurosky.com/docs/doku.php?id=thinkgear_communications_protocol
    


    TODO - GNU license etc

*/

#include <sys/time.h> // json timestamp
#include <time.h>
#include <json-c/json.h>
#include "nsconn.h"

// ThinkGear data valaues
#define RAWWAV        0x80
#define RAWPLENGTH    0x02
#define ASICEEGPOWER  0x83
#define LOWPOWER      0x02
#define ATTENTION     0x04
#define MEDITATION    0x05


extern conf_t runConfig;


// create unix timestamp for object
unsigned long getTime(){

    struct timeval tv;

    gettimeofday(&tv, NULL);

    unsigned long long currTime =
        (unsigned long long)(tv.tv_sec) * 1000 +
        (unsigned long long)(tv.tv_usec) / 1000;

   return currTime;  // return timestamp in milliseconds 

}

int jsonFormat(unsigned char * payload, int sz){

    short raw = 0;
    unsigned char attention = 0;
    unsigned char meditation = 0;
    unsigned char poorsignal = 0;
    unsigned long creationTime = 0;

  

    //printf("starting payload parse\n");
    for (int byte = 0; byte < sz; byte++){

        if ((payload[byte] == RAWWAV) && (payload[byte+1] == RAWPLENGTH)) {  // pLength should always be 0x02
        
        // if raw wave output is not disabled
        if(runConfig.rawwavOff == false) {
         
            errno = 0;
            json_object *rawwavObj = json_object_new_object();

            byte = (byte+2); // skip next byte (we know it's 0x02)
            raw = (payload[byte] << 8) | payload[byte+1];


            // add value & time to object
            errno = 0;
            if(json_object_object_add(rawwavObj, "raweegpower", json_object_new_int(raw)) != 0){
                
                fprintf(stderr, "raweegpower fail \nerror: %s\n", strerror(errno));

            }

            errno = 0;
            if(json_object_object_add(rawwavObj, "time", json_object_new_int64(getTime())) != 0){

                fprintf(stderr, "raweeg (time) fail \nerror: %s\n", strerror(errno));

            }

            byte += 2;

            // send object string to forward()
            forward(json_object_to_json_string_ext(rawwavObj, JSON_C_TO_STRING_SPACED));

        } else { byte = byte+4; }  // if rawwav disabled

    } else if (payload[byte] == ASICEEGPOWER){

        if((payload[byte+1]) == 24){  // check payload is correct length

            byte = (byte+2); // skip payload position to 1st value byte


            // asic object
            json_object *asicObj = json_object_new_object();


            // create array object to hold 8 signal bands
            json_object *jValsArray = json_object_new_array();


            // increments three bytes per loop until all values are parsed
            for(int j = 0; j < 24; j = (j+3)){
               

                // DO NOT TOUCH THIS AGAIN IDIOT
                unsigned char b1 = payload[byte];
                unsigned char b2 = payload[byte+1];
                unsigned char b3 = payload[byte+2];

                // concatenate 3 bytes to get bigendian eeg value
                uint32_t asicVal = (b1 << 16) | (b2 << 8) | b3;

               


                json_object *jVal = json_object_new_int(asicVal);   

                errno = 0;
                if(json_object_array_add(jValsArray,jVal) != 0){

                    fprintf(stderr, "asic array add fail \nerror: %s\n", strerror(errno));  

                } 


                byte = byte+3;  // increase number of bytes parsed in payload loop

            } // end band value collection
            // add collected values to root object
            errno = 0;
            if(json_object_object_add(asicObj,"asiceegpower", jValsArray) != 0){

                 fprintf(stderr, "asiceegpower fail \nerror: %s\n", strerror(errno));

            } 
            
            errno = 0;
            if(json_object_object_add(asicObj, "time", json_object_new_int64(getTime())) != 0){
                
                fprintf(stderr, "asiceegpower (time) fail \nerror: %s\n", strerror(errno));

            }
          
             forward(json_object_to_json_string_ext(asicObj, JSON_C_TO_STRING_SPACED));    

        }
        

    } else if(payload[byte] < RAWWAV) {

        if (payload[byte] == LOWPOWER){

            json_object *poorsigObj = json_object_new_object();
            poorsignal = payload[byte+1];

            errno = 0;
            if((json_object_object_add(poorsigObj, "poorsignal", json_object_new_int(poorsignal))) || (json_object_object_add(poorsigObj, "time", json_object_new_int64(getTime()))) == 1 ){
 
                fprintf(stderr, "poorsignal fail \nerror: %s\n", strerror(errno));

            }

            errno = 0;
            if(json_object_object_add(poorsigObj, "time", json_object_new_int64(getTime())) != 0){

                fprintf(stderr, "poorsignal (time) fail \nerror: %s\n", strerror(errno));

            }
            byte++;  
            forward(json_object_to_json_string_ext(poorsigObj, JSON_C_TO_STRING_SPACED));


        } else if (payload[byte] == ATTENTION){

            json_object *attObj = json_object_new_object();

            attention = payload[byte+1];

            errno = 0;
            if(json_object_object_add(attObj, "attention", json_object_new_int(attention)) != 0){

                fprintf(stderr, "attention fail \n error: %s\n", strerror(errno));

            }

            errno = 0;
            if(json_object_object_add(attObj, "time", json_object_new_int64(getTime())) != 0){

                fprintf(stderr, "time fail \n error: %s\n", strerror(errno));

            }

            byte++;
            forward(json_object_to_json_string_ext(attObj, JSON_C_TO_STRING_SPACED));

        } else if (payload[byte] == MEDITATION){

            json_object *medObj = json_object_new_object();
            meditation = payload[byte+1];

            errno = 0;
            if(json_object_object_add(medObj, "meditation", json_object_new_int(meditation)) != 0){

                fprintf(stderr, "meditation fail \nerror: %s\n", strerror(errno));

            }

            errno = 0;
            if(json_object_object_add(medObj, "time", json_object_new_int64(getTime())) != 0){

                fprintf(stderr, "meditation (time) fail \nerror: %s\n", strerror(errno));

            }
            byte++;
           
            forward(json_object_to_json_string_ext(medObj, JSON_C_TO_STRING_SPACED));

        }
      }

    }

    return 0;

}



