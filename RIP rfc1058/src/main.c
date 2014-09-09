#include "library.h"
#include "library.c"

int main(int argc, char const *argv[])
{
	
	if (argc != 2){
		printf("usage: ./whatever xxx.xxx.xxx.xxx\n");
		exit(1);
	}	

	FILE *fp = fopen("neighbor.config","r");
	char *dest; //for getline
	dest = calloc(1,GETLINE_MAX);

	int i,ntableSize,vtableSize;
	struct ntableEntry *ntable[MAX_NTABLE];
	struct vtableEntry *vtable[MAX_VTABLE];	
	struct rtableEntry rtable[MAX_RTABLE];


	int fd; //my socket fd
	struct sockaddr_in *me;
	struct in_addr my_vip; //transform from argv[1]
	char str[INET_ADDRSTRLEN]; // print out IP adr

	struct sockaddr_in *narray[MAX_NEIGHBOR_PER_NODE];
	int narraySize=0; //will be update during searching neighbors
	char me_ip[INET_ADDRSTRLEN];
	char ip_temp[INET_ADDRSTRLEN];


	unsigned int sockaddr_in_len; //used in sendto()
	sockaddr_in_len = sizeof(struct sockaddr_in);

	int rv, n; //for select
	fd_set readfds,tempfds;
	struct timeval tv;

	int recvlen; //# of reveived bytes 
	
	/*--------------------------------------setup ntable*/
	for (i=0;i<MAX_NTABLE;i++){
		ntable[i] = NULL;
	}

	for(i = 0;getLine(dest,fp) != 0; i++)
	{
		ntable[i] = make_ntableEntry(dest);
	}
	ntableSize = i;
	// printf("ntable size=%d\n",ntableSize );

	/*--------------------------------------setup vtable*/
	fp = fopen("node.config","r");
	for (i=0;i<MAX_VTABLE;i++){
		vtable[i] = NULL;
	}

	for(i = 0;getLine(dest,fp) != 0; i++)
	{
		vtable[i] = make_vtableEntry(dest);
	}
	vtableSize = i;
	// printf("vtable size=%d\n",vtableSize );

	fclose(fp);

	/*--------------------------------------test seaching*/
	// for(i =0; i < ntableSize; i++){
	// 	print_sockaddr(search_vtable(ntable[i]->nvip_addr, &vtable, vtableSize)	);
	// }

	/*--------------------------------------------------------create socket*/ 
	if ((fd = socket(AF_INET, SOCK_DGRAM,0))<0) {
		perror("connot create socket");
		return 0;
	}
	// printf("created socket: descriptor = %d\n", fd);


	/*--------------------------------------------------------bind*/
	//fill in sockaddr info	

	if (inet_pton(AF_INET,argv[1], &my_vip)!=1){ //assign IP adr myself
		printf("inet_pton() failed @myaddr\n");
	}; 
	me = search_vtable(my_vip,&vtable,vtableSize);

	if (bind(fd, (struct sockaddr *)me, sizeof(*me)) < 0) {
		perror("bind failed");
		return 0;
	}

	//print result
	inet_ntop(AF_INET, &(me->sin_addr),str, INET_ADDRSTRLEN);
	printf("bind complete. IP: %s @%d\n", str,ntohs(me->sin_port));

	/*--------------------------------------------------------get neighbor array*/
	for (i=0;i<MAX_NEIGHBOR_PER_NODE;i++){
		narray[i] = NULL;
	}

	inet_ntop(AF_INET, &my_vip,me_ip, INET_ADDRSTRLEN); //fill me_ip

	for(i=0;i<ntableSize;i++){
		inet_ntop(AF_INET, &(ntable[i]->vip_addr),ip_temp, INET_ADDRSTRLEN);

		// printf("comparing: (%s) <> (%s)\n",me_ip,ip_temp );
	
		if (strcmp(me_ip,ip_temp) == 0){
			narray[narraySize] = search_vtable(ntable[i]->nvip_addr,&vtable, vtableSize);
			narraySize++;
		}
	}

	if (narraySize>0){
		printf("I have %d neighbor as follow:\n",narraySize );

		for(i=0;i<narraySize;i++){
			printf("\t");
			print_sockaddr(narray[i]);
		}
	}
	else
		printf("I have no neighbor orz\n");

	/*------------------------------------initialize routing table*/
	// for(i =0; i< MAX_RTABLE; i++){
	// 	memset(&rtable[i],0,sizeof(struct rtableEntry));
	// }

	for(i=0;i<narraySize;i++){
		rtable[i].destination = narray[i];
		rtable[i].gateway = me;//narray[i];
		rtable[i].metric = 1;
		rtable[i].timeout = time(NULL);
		rtable[i].gc = time(NULL);
		rtable[i].flag_exist = 1;
	}

	// print_allrtable(&rtable);	

	/*------------------------------------send & recv msg*/
	time_t update_timer=time(NULL);
	short ok=0;

	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	FD_SET(fd, &readfds);
	n = fd +1;

	tv.tv_sec = 3;
	tv.tv_usec =0;


	while(1){
		for(i=0;i<MAX_RTABLE;i++){
			if( timer_checker(rtable[i].timeout,EXPIRE_TIME)){
				set_flag(&rtable[i].flag_invalid,1);
				timer_reset(&rtable[i].timeout);
			}

			if(timer_checker(rtable[i].gc,GC_TIME)){
				if(rtable[i].flag_invalid){
					set_flag(&rtable[i].flag_exist, 0);
					set_flag(&rtable[i].flag_invalid,0);
					timer_reset(&rtable[i].gc);
				}
			}
		}


		if (timer_checker(update_timer,UPDATE_TIME)){ //3 sec to send msg out
			set_flag(&ok,1);
		}
		if(ok){
			

			// sprintf(sendbuf,"Hello,there!");
			for(i=0;i<narraySize;i++){
				struct rmsg out_msg;
				make_msg(&rtable, &out_msg, narray[i]);

				if (sendto(fd,&out_msg,sizeof(out_msg),0,(struct sockaddr *)narray[i],sockaddr_in_len) == -1){
					printf("err @sendto\n");
					exit(1);
				}
				// printf("sending RIP msg(size:%lu) to ",sizeof(out_msg));
				// print_sockaddr(narray[i]);
			}
			// printf("-----------------------------\n\n");

			timer_reset(&update_timer);
			set_flag(&ok,0);
		}
		
		FD_ZERO(&tempfds);
		tempfds = readfds;

		rv = select(n, &tempfds, NULL,NULL, &tv);

		if(rv ==-1)
			perror("select");
		else if(rv ==0){
			// printf("timeout @select\n");
		}
		else{
			if (FD_ISSET(fd, &tempfds)){
				struct rmsg *in_msg;
				in_msg = calloc(1,sizeof(struct rmsg)); //those two lines are crucial key for wired bugs, though I still don't know why

				struct sockaddr_in who_send_me;
				
				recvlen = recvfrom(fd, in_msg, sizeof(struct rmsg), 0, (struct sockaddr *)&who_send_me,&sockaddr_in_len);
			    if (recvlen >= 0) {
		            int msg_table_size = in_msg->arraySize;

		            for(i=0;i<msg_table_size;i++){ //go through every entry in the msg
		            	
		            	if(in_msg->array[i].metric > 15) //ingore infinity msg entry
		            		continue;

		            	// if(is_neighbor(&narray, narraySize, &in_msg->array[i].destination )){ //if destination is my neighbor, ignore
		            	// 	continue;
		            	// }

		            	if(is_sockaddr_same(me, &in_msg->array[i].destination ) ==1 ){ //if destination is myself, ignore
		            		continue;
		            	}

		            	int new_metric;
		            	new_metric = 1 + in_msg->array[i].metric;
		            			            	
		            	int target_slot;
		            	target_slot = is_exist_in_rtable(&rtable,&(in_msg->array[i].destination) );


		            	if(target_slot >= 0){ //destination exist in rtable

		            		if( new_metric < rtable[target_slot].metric){ //better route found. update gateway and metric, printout new rtable
		            			printf("received RIP msg(size:%d) from: ",recvlen);
		            			print_sockaddr(&who_send_me);

		            			printf("msg[%d]:",i);
		            			print_rmsgEntry(in_msg->array[i]);

		            			printf("updated routing table entry[%d]:\n",target_slot);

		            			printf("\told entry is:\n");
		            			print_rtableEntry(rtable[target_slot]);
		            			printf("\n\n");


		            			rtable[target_slot].gateway = &who_send_me;
		            			/*update gateway, the reason why I can't just assigh &who_send_me is I need to use pointer in vtable*/

		            			rtable[target_slot].metric = new_metric;
		            			timer_reset(&rtable[target_slot].timeout);
		            			
		            			
		            			printf("\tnew entry would be:\n");
		            			print_rtableEntry(rtable[target_slot]);
		            			printf("\n\n");
		            			print_allrtable(&rtable);

		            		}
		            		else{
		            			// if(is_sockaddr_same(&who_send_me, rtable[target_slot].gateway) == 1){ //not better route, but sender is gateway of this route, update timeout
		            				timer_reset(&rtable[target_slot].timeout);
		            			// }
		            		}
		            	}
		            	else{ //no exist -> create new entry in rtable, printout new rtable
		            		int new_slot = find_empty_slot(&rtable);

		            		if(new_slot >= 0 ){ //I can find an empty slot in rtable
		            			
		            			rtable[new_slot].destination = &in_msg->array[i].destination;  //possible bug, transform from message
		            			rtable[new_slot].gateway = &who_send_me;  /*search_vtable(who_send_me.sin_addr,&vtable, vtableSize);*/ 
		            			rtable[new_slot].metric = new_metric;
		            			rtable[new_slot].timeout = time(NULL);
		            			rtable[new_slot].gc = time(NULL);
		            			rtable[new_slot].flag_exist = 1;

		            			printf("\n\nreceived RIP msg(size:%d) from: ",recvlen);
		            			print_sockaddr(&who_send_me);
		            			printf("msg[%d]:",i);
		            			print_rmsgEntry(in_msg->array[i]);

		            			printf("create new entry in routing table slot[%d]:\n", new_slot);
		            			printf("\tnew entry would be:\n");
		            			print_rtableEntry(rtable[new_slot]);
		            			printf("\n\n");
		            			print_allrtable(&rtable);
		            		}
		            	}
		            }
			    }
			}
		}
	}
	return 0;
}














