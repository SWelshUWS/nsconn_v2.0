/*

    nsconn outsock.c
    handles the network connection to clients by setting
    up and maintaining sockets on 127.0.0.1:9192.
    nsconn only supports one client at a time. 

    also
    thank u beej


*/
// netsock headers
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


#include "nsconn.h"

#define BACKLOG 2	 // backlog count is 2

extern	socks_t	socks;
extern	conf_t	runConfig;


void sigchld_handler(int warning) {

	(void)warning; 

	// save errno incase of overwrite elsewhere
	int errnos = errno;

    // wait for the child process to die
    while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = errnos;
}


// get client address (IPv4/6)
void *getClientAddr(struct sockaddr *addr) {

	if (addr->sa_family == AF_INET) {
	    return &(((struct sockaddr_in*)addr)->sin_addr);
    }
	return &(((struct sockaddr_in6*)addr)->sin6_addr);
}


void sockStart(void) {

  
	struct addrinfo defaddr, *servinfo, *sp;
	struct sockaddr_storage clientAddr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int sockcheck;

    // hold default values (localhost, TCP etc etc)
	memset(&defaddr, 0, sizeof defaddr);
	defaddr.ai_family = AF_UNSPEC;    // both ipv4 or ipv6
	defaddr.ai_socktype = SOCK_STREAM;    // use TCP
	defaddr.ai_flags = AI_PASSIVE; // use my IP

    // if we cannot get socket info
	if ((sockcheck = getaddrinfo(NULL, runConfig.portOut, &defaddr, &servinfo)) != 0) {
		
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(sockcheck));
		exit(1);

	}

    errno = 0;
	// loop through all the results and bind to the first we can
	for(sp = servinfo; sp != NULL; sp = sp->ai_next) {

		if ((socks.outsock = socket(sp->ai_family, sp->ai_socktype,
				sp->ai_protocol)) == -1) {

			fprintf(stderr, "server: socket %s\n", strerror(errno));
			continue;

		}

        errno = 0;
		if (setsockopt(socks.outsock, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {

			fprintf(stderr, "setsockopt: %s\n", strerror(errno));
			exit(1);

		}

        errno = 0;
		if (bind(socks.outsock, sp->ai_addr, sp->ai_addrlen) == -1) {

			close(socks.outsock);
			fprintf(stderr, "server: bind %s\n", strerror(errno));
			exit(1);

		}

		break;

	}

	freeaddrinfo(servinfo); // all done with this structure

    errno = 0;
	if (sp == NULL)  {

		fprintf(stderr, "failed to bind... %s\n", strerror(errno));
		exit(1);

	}

    errno = 0;
	if (listen(socks.outsock, BACKLOG) == -1) {

		fprintf(stderr, "unable to listen for connections... %s\n", strerror(errno));   
		exit(1);

	}

    errno = 0;
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {

		fprintf(stderr, "sigaction fail... %s\n", strerror(errno));
		exit(1);

	}

	printf("nsconn on 127.0.0.1:%s - waiting for connections...\n", runConfig.portOut);

}

// listen for connections
void listenForClients(void){


    struct sockaddr_storage clientAddr; // connector's address information
	socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

        errno = 0;
		sin_size = sizeof clientAddr;
		socks.clientsock = accept(socks.outsock, (struct sockaddr *)&clientAddr, &sin_size);
		if (socks.clientsock == -1) {

			fprintf(stderr, "unable to accept connection... \n error: %s\n", strerror(errno));
			exit(1);
			
		}

		inet_ntop(clientAddr.ss_family,
			getClientAddr((struct sockaddr *)&clientAddr),
			s, sizeof(s));

		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process

            // clean up listener
			close(socks.outsock); 

            // start parsing packets 
            parsePackets();    // TODO - provide a return value for errors
           
            // close socket if still open when parsePackets() terminates
			close(socks.clientsock);
			exit(0);

		}

		close(socks.clientsock);  // parent doesn't need this
	

}