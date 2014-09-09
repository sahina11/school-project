#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "dns.h"

int main(int argc, char const *argv[])
{
	int sockfd, new_fd; //listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo;
	int rv; //return value for error checking 

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin); //these two for print out port#

	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char ipv4[INET_ADDRSTRLEN]; //for print out ip addr

	struct nameToIP *dictionary = NULL;
	struct msg msgIn; //for incoming msg
	memset(&msgIn, 0, sizeof(msgIn));


  //--------------------------------------------------- code started 
	memset( &hints, 0, sizeof(hints)); //preparation
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, "0", &hints, &servinfo)) != 0) { //set port# 0 to find an available port
		printf("error@server: getaddrinfo\n");
		return 1;
	}

	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, 
		servinfo->ai_protocol)) == -1) {
		printf("error@server: socket fd\n");
		return 1;
	}

	if ((rv = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		close(sockfd);
		printf("error@server: bind\n");
		return 1;
	}

	if (listen(sockfd, BACKLOG) == -1) {
		printf("error@server: listen\n");
		return 1;
	}

	getsockname(sockfd, (struct sockaddr *)&sin, &len);
	printf("Available port number = %u\n", ntohs(sin.sin_port));
	printf("server: waiting for connections...\n");

	while(1){
		sin_size = sizeof their_addr;

		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
			// printf("error@server: accept\n");
			continue;
		}

		inet_ntop(
			their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			ipv4, sizeof ipv4);
		
		printf("server: got connection from %s\n", ipv4);

		if(recv(new_fd, &msgIn, sizeof(msgIn), 0) == -1){ //server recv the msgIn
			printf("error@server: recv\n");
			return 1;
		}
		else{ //if no error of recv msg, start to process msg and reply
			if (msgIn.type == REGISTER){ //handle register msg
				
				struct nameToIP *newEntry;
				newEntry = calloc(1, sizeof(struct nameToIP));
				strcpy(newEntry->domainName, msgIn.domainName);
				strcpy(newEntry->ip, msgIn.ip);
				newEntry->next = NULL;

				if(dictionary == NULL) {
					dictionary = newEntry; //if this is the first entry
					printf("server: new entry created.\n \tdomainName=%s\n \tIP=%s\n",
						newEntry->domainName, newEntry->ip);
				}
				else {
					insertNewEntry(dictionary, newEntry);
					printf("server: new entry created.\n \tdomainName=%s\n \tIP=%s\n",
						newEntry->domainName, newEntry->ip);
				}

				close(new_fd);
				continue;
			}
			else if(msgIn.type == GETIP){ //handle GETIP request
				char *ipTobe;
				ipTobe = findIP(dictionary, msgIn.domainName);
				if(ipTobe ==NULL){
					sendErrorMsg(new_fd, "Server: Couldn't find the IP address");
					
					close(new_fd);
					continue;
				}
				else{
					sendIPMsg(new_fd, ipTobe);
					
					close(new_fd);
					continue;
				}
			}
			else{ //handle error msg
				printf("error@server: recv, get an unknow msg from client: %s\n", ipv4);

				close(new_fd);
				return 1;
			}
		}

		close(new_fd);
	}

	return 0;
}









