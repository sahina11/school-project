#define BACKLOG 10
#define MAXDATASIZE 100 // max number of bytes we can get at once 

#define REGISTER 100
#define GETIP 101
#define ERROR_MSG 102

struct msg {
	int type;
	char domainName[MAXDATASIZE];
	char ip[MAXDATASIZE];
	char errorMsg[MAXDATASIZE];
};

struct nameToIP{
	char domainName[MAXDATASIZE];
	char ip[MAXDATASIZE];
	struct nameToIP *next;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


char *findIP(struct nameToIP *dictionary, char *domainName){
	if (dictionary == NULL)
		return NULL;
	else{
		if (strcmp(dictionary->domainName, domainName) == 0){
			return dictionary->ip;
		}
		else return findIP(dictionary->next, domainName);
	}
}

int insertNewEntry(struct nameToIP *dictionary, struct nameToIP *entry){
	if (dictionary == NULL) return -1;
	else{
		if (dictionary->next ==NULL) {
			dictionary->next = entry;
			return 0;
		}
		else return insertNewEntry(dictionary->next, entry);
	}
}

int sendErrorMsg(int sockfd, char *msg){
	struct msg msgOut;
	memset(&msgOut, 0, sizeof(msgOut));

	msgOut.type = ERROR_MSG;
	strcpy(msgOut.errorMsg, msg);

	return send(sockfd, &msgOut, sizeof (msgOut), 0);
}

int sendIPMsg(int sockfd, char *ip){
	struct msg msgOut;
	memset(&msgOut, 0, sizeof(msgOut));

	msgOut.type = GETIP;
	strcpy(msgOut.ip, ip);

	return send(sockfd, &msgOut, sizeof (msgOut), 0);
}


int sendRegisterReq(int sockfd, char *domainName, char *ip){
	struct msg msgOut;
	memset(&msgOut, 0, sizeof(msgOut));

	msgOut.type = REGISTER;
	strcpy(msgOut.domainName, domainName);
	strcpy(msgOut.ip, ip);

	return send(sockfd, &msgOut, sizeof (msgOut), 0);
}

int sendGetIPReq(int sockfd, char *domainName){
	struct msg msgOut;
	memset(&msgOut, 0, sizeof(msgOut));

	msgOut.type = GETIP;
	strcpy(msgOut.domainName, domainName);

	return send(sockfd, &msgOut, sizeof (msgOut), 0);
}







