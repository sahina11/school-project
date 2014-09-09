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
	if (argc != 4){
		printf("usage: dnsGetIP hostname||IP_addr port# domain_name\n");
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

	sendGetIPReq(sockfd, (char *)argv[3]);

	if(recv(sockfd, &msgIn, sizeof(msgIn), 0) == -1){ //client recv the msgIn
		printf("error@client: recv\n");
		
		close(sockfd);
		return 1;
	}
	else{
		if (msgIn.type == ERROR_MSG){
			printf("%s\n",msgIn.errorMsg);

			close(sockfd);
			return 1;
		}
		else if(msgIn.type == GETIP){
			printf("server: IP address of domain name(%s) is %s\n",argv[3],msgIn.ip);

			close(sockfd);
			return 0;
		}
		else{
			printf("server: what the hell you get back to me ???\n");

			close(sockfd);
			return 1;			
		}
	}
}













