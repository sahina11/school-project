#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// #include <sys/wait.h>
// #include <signal.h>

#include "dns.h"

int main(int argc, char const *argv[])
{
	int sockfd; 
	struct addrinfo hints, *servinfo;
	int rv; //return value for error checking 

	char ipv4[INET_ADDRSTRLEN]; //for print out ip addr

	struct msg msgIn; //for incoming msg
	memset(&msgIn, 0, sizeof(msgIn));

	//--------------------------------------------------- code started 
	if (argc != 5){
		printf("usage: dnsReg hostname||IP_addr port# domain_name IP_addr\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) { //use argv2 to setup port#
		printf("error@client: getaddrinfo\n");
		return 1;
	}

	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, 
		servinfo->ai_protocol)) == -1) {
		printf("error@client: socket fd\n");
		return 1;
	}

	if ((rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		close(sockfd);
		printf("error@client: connect\n");
		return 1;
	}

	inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr),
			ipv4, sizeof ipv4);
	
	printf("client: connecting to %s\n", ipv4);
	
	if (sendRegisterReq(sockfd, (char *)argv[3], (char *)argv[4]) == -1){
		printf("client: fail to send msg\n");

		close(sockfd);
		return 1;
	}


	close(sockfd);
	return 0;
}













