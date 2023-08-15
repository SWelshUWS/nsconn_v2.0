/*

    nsconnn v2.0 main file
    collects user options and configures nsconn before
    handling connections to clients and the headset


    TODO - change getopts for argp as it's cleaner and closer to UNIX standard

*/

#include "nsconn.h"
#include <glib.h>    // noone else uses this


socks_t     socks;      // all sockets in use
conf_t      runConfig;  // intialize configuration struct
conf_t      *pRunConf = &runConfig;



int main(int argc, char *argv[]){

    
    int count;          // counter
    int option;     // parse CLI args


    // initialize all configurations as off 
    runConfig.output = false;  // print payload to stdout
    runConfig.netOut = false; // select socket type
    runConfig.rawwavOff = false; // disable raw eeg wave output
    runConfig.format = JSON;
    runConfig.portOut = "";
    runConfig.dest = "";


    // config file values 
    GKeyFile *keyfile;
    GKeyFileFlags flags;
    GError *error = NULL;
    gchar *defaultMAC, *defaultPort;
    const char * confpath = "/etc/nsconn.conf"; // change me before push to /etc/nsconn


    // text to display when -h specified
    const char * helpText = "Basic Usage:\n"
        "nsconn [options] [MAC address]\n\n"
        "\t -h: \tdisplay basic usage info and exit.\n"
        "\t -v: \tshow display version and exit\n"
        "\t -d: \tdisable raw wave output\n"
        "\t -o: \tenable stdout\n"
        "\t -n: \tenable output to network socket\n\n"
        "\t -f [json|binary]: \tspecify output format \n"
        "\t -p [port]: \t\tspecify which outbound port to bind to\n\n"
        "configuration file: /etc/nsconn.conf\n\n";


    // splash on start
    printf("\n    _   _______ __________  _   ___   __ \n"
        "   / | / / ___// ____/ __ \\/ | / / | / / \n"
        "  /  |/ /\\__ \\/ /   / / / /  |/ /  |/ / \n"
        " / /|  /___/ / /___/ /_/ / /|  / /|  /  \n"
        "/_/ |_//____/\\____/\\____/_/ |_/_/ |_/   \n"
        "\nnsconn - a connector for the neurosky mindwave mobile 2\n\n");


    // Create a new GKeyFile object and a bitwise list of flags.
    keyfile = g_key_file_new ();
    flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
    
    // load configuration file
    if(!g_key_file_load_from_file (keyfile, confpath, flags, &error)){

        // handle non existant config file
        g_printerr("could not load configuration file.\n check that /etc/nsconn.conf exists.\nerror: %s\n",error->message); 
        g_error_free(error);
        exit(0);

    } 

    
    if(argc > 1){    // if arguments are provided

        while((option = getopt(argc, argv, ":p:f:c:dovnh")) != -1){ //get option from the getopt() method

            switch(option){

                // options with no value
                case 'h':
                    printf("%s", helpText);
                    exit(0);
                    break;

                case 'v':
                    printf("%s\n", "nsconn v2.0 - 2022");
                    exit(0);
                    break;

                case 'o':
                    // enable printing final payload to stdout
                    printf("%s\n", "printing to stdout enabled...\n");
                    runConfig.output = true;
                    break;

                case 'n':
                    // netsocket out should only start when selected
                    runConfig.netOut = true;
                    printf("%s\n", "network socket selected...");
                    break;
        
                case 'd':      // override default config port
                    runConfig.rawwavOff = true;
                    printf("%s\n", "raw EEG wave value disabled... ");
                    break;


                // options expecting values

                case 'p':      // override default config port
                    // protect against buffer overflows with strcpy
                    if (optarg == NULL) {
                        exit(0);
                    } else if (strlen(optarg) > 5) {
                     // port should never be more than 5 characters
                        printf("%s\n", "Port number too long.");
                        exit(0);
                    } else {    // TODO snprintf() port arg                  
                        runConfig.portOut = optarg;
                    }
                    printf("user defined port: %s\n", runConfig.portOut);
                    break;

                case 'f':      // change format of output
                    if(strcmp(optarg, "json") == 0){
                        runConfig.format = JSON;
                        printf("JSON output selected\n");
                    } else if(strcmp(optarg, "binary") == 0){
                        runConfig.format = BINARY;
                        printf("binary output selected.\n");
                    }
                    break;
         
                case 'c':      // set the running config
                    /*
                        currently not used
                        this would allow the user to set their own configuration file
                    */
                    printf("running config: %s\n", optarg);
                    break;

                case ':':
                printf("option needs a value\n");
                break;
    
                case '?':      //used for some unknown options
                    printf("invalid option selected: %c\n", optopt);
                    exit(0);
                    break;
                }
            }
        
        // parse for MAC address
        if (argv[optind]){    // if there are more arguments

            for(; optind < argc; optind++){ //when some extra arguments are passed

                if (!argv[optind+1]){   // if this is the last value

                    if (strlen(argv[optind]) == 17){ // if the provided argument is the correct length   

                        runConfig.dest = argv[optind]; 
                        if (strlen(runConfig.dest) != 17){    // ensure MAC is correct length

                            printf("supplied MAC not 17 characters.\n");  
                            exit(0);     

                        } else {

                            defaultMAC = g_key_file_get_string(keyfile,"bluetooth","DEFAULT_MAC",&error);    // TODO use error here
                            if(strlen(defaultMAC) < 17){

                                printf("no default MAC exists. add this MAC to config file? Y/n\n");
                                char atc = 'y';
                                scanf("%c", &atc);

                                if ( tolower(atc) == 'y' ){    // TODO make sure this works

                                    printf("saving MAC to config...\n");
                                    g_key_file_set_string(keyfile,"bluetooth","DEFAULT_MAC",runConfig.dest);                  
                                    printf("default MAC: %s", defaultMAC);
                                    if(g_key_file_save_to_file(keyfile, confpath, &error) == 0){
                                        g_printerr("could not write to file.\nerror: %s\n",error->message); 
                                        g_error_free(error);
                                        exit(0);

                                    } 
                                }
                            } 
                        }
                    }
                } else {

                    // display help and exit
                    printf("too many arguments\n%s", helpText);
                    exit(0);
                }
            }
        }
    } 
    
    /*
        end gathering arguments -
        check which struct elements are still null
        and fill from the config file
    */
    
    if(runConfig.dest == ""){ // if MAC not set
        
        if(defaultMAC = g_key_file_get_string(keyfile,"bluetooth","DEFAULT_MAC",&error)){

            errno = 0;
            if (strlen(defaultMAC) == 17){
                printf("%s\n", defaultMAC); 
                runConfig.dest = defaultMAC;

            } else {
                g_printerr("could not get port from configuration file.\nerror: %s\n",error->message);
                g_error_free(error);
                exit(0);

            }
        }
    } 
    if(strlen(runConfig.portOut) < 1){

        errno = 0;
        if((defaultPort = g_key_file_get_string(keyfile,"network","PORT",&error)) < 0){

            g_printerr("could not get port from configuration file.\nerror: %s\n",error->message);
            g_error_free(error);
            exit(1);

        }

        runConfig.portOut = defaultPort;


    }    // end start config
                      
   

   // if no network socket
    if(runConfig.netOut == false){

        if(runConfig.output == true){
            
            printf("printing to stdout...");
            nsConnect();
            parsePackets(); // start parsing packets to stdout

        } else {

            printf("no output method selected\n");
            exit(0);

        }
    } else {
    
        nsConnect();
        sockStart();
        listenForClients();

    }

    printf("Exiting nsconn...\n");


    return 0;
}
