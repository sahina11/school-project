#include <stdlib.h>	/* defines system calls */
#include <stdio.h>	/* needed for printf */
#include <string.h>	/* needed for memset */
#include <sys/socket.h>
#include <netinet/in.h>	/* needed for sockaddr_in */
#include <arpa/inet.h> /*needed for inet_ntop*/
#include <unistd.h> /*needed for close()*/
#include <sys/select.h> /*needed for select()*/
#include <math.h> /*for pow*/
#include <signal.h> /*for alarm*/
#include <time.h> 

#define IP_ADDR "127.0.0.1"

#define GETLINE_MAX 50
#define MAX_VTABLE 30
#define MAX_NTABLE 30
#define MAX_NEIGHBOR_PER_NODE 10
#define MAX_RTABLE 10


#define EXPIRE_TIME 18
#define UPDATE_TIME 3
#define GC_TIME 12



struct vtableEntry{ 
	/*basically, all the sockaddr_in are stored in this table
	the rest of the program only use its pointer*/
	struct in_addr vip_addr;
	struct sockaddr_in sockaddr;
};

struct ntableEntry{
	struct in_addr vip_addr;
	struct in_addr nvip_addr;
};

struct rtableEntry{
	struct sockaddr_in *destination;
	struct sockaddr_in *gateway;
	int metric;
	
	time_t timeout;
	time_t gc;

	short flag_invalid; //for expiration
	short flag_exist;//for gargabe collection
};

struct rmsgEntry{
	struct sockaddr_in destination;
	int metric;
};

struct rmsg{
	int arraySize;
	struct rmsgEntry array[MAX_RTABLE];
};






