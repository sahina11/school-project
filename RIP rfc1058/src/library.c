int getLine (char *dest,FILE *fp){
	if (fgets(dest, GETLINE_MAX, fp) == NULL)
		return 0;
	else{
		if (*dest == '#'){
			return getLine(dest, fp);
		}
		else
			return strlen(dest);
	}	
}

short stoi (char *str){
	short sum,i,power;
	sum = 0;
	power = strlen(str)-1;

	for (i = 0; i < strlen(str); i++,power--){
		sum += pow(10,power) * (*(str+i)-'0');
	}
	return sum;
}

struct vtableEntry *make_vtableEntry(char *line){

	struct vtableEntry *res;
	res = calloc(1,sizeof(struct vtableEntry));
	char *token;
	short port;
	int i; //to get rid of last \n char of last token


	token = strtok(line," "); //vip
	if (inet_pton(AF_INET,token, &(res->vip_addr))!=1){ //assign IP adr myself
		printf("inet_pton() failed @res->vip_addr\n");
	}; 
	
	token = strtok(NULL," "); // port#
	port = stoi(token);
	res->sockaddr.sin_port = htons(port);

	token = strtok(NULL," "); // real ip
	
	i = strpbrk(token,"\n") - token; //get rid of last \n char
	*(token+i) = 0;

	if ((i =inet_pton(AF_INET,token, &(res->sockaddr.sin_addr)))!=1){ //assign IP adr myself
		printf("return: %d, inet_pton() failed @res->sockaddr.sin_addr\n",i);
	}; 

	res->sockaddr.sin_family = AF_INET; //fill in familiy

	return res;
}

void print_vtableEntry(struct vtableEntry *this){
	if(this == NULL){
		printf("err @print_vtableEntry for null pointer\n");
		exit(1);		
	}
	char str1[INET_ADDRSTRLEN];
	char str2[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(this->vip_addr),str1, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(this->sockaddr.sin_addr),str2, INET_ADDRSTRLEN);
	printf("virtual IP: %s --> IP:%s @%d\n",str1,str2,ntohs(this->sockaddr.sin_port));

}

struct sockaddr_in *search_vtable(struct in_addr vip, 
	struct vtableEntry *(*vtable)[], int size){
		
	char vip_input[INET_ADDRSTRLEN];
	char vip_inTable[INET_ADDRSTRLEN];
	char hit[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &vip,vip_input, INET_ADDRSTRLEN);

	int i;
	for(i=0;i<size;i++){
		// printf("searching entry: ");
		// print_vtableEntry((*vtable)[i]);
		
		inet_ntop(AF_INET, &(*vtable)[i]->vip_addr,vip_inTable, INET_ADDRSTRLEN);

		if (strcmp(vip_inTable, vip_input)==0){
			// printf("got you! %s -->",vip_input);
			inet_ntop(AF_INET, &(*vtable)[i]->sockaddr.sin_addr,hit, INET_ADDRSTRLEN);
			// printf("%s @%d\n",hit,ntohs((*vtable)[i]->sockaddr.sin_port) );
			return &((*vtable)[i]->sockaddr);
		}

	}
	printf("didn't found you:%s\n",vip_input );
	return NULL;

}

struct ntableEntry *make_ntableEntry(char *line){
	struct ntableEntry *res;
	res = calloc(1,sizeof(struct ntableEntry));
	char *token;
	int i;

	token = strtok(line," "); //vip
	if (inet_pton(AF_INET,token, &(res->vip_addr))!=1){ //assign IP adr myself
		printf("inet_pton() failed @res->vip_addr\n");
	}; 

	token = strtok(NULL," "); //vnip
	if ((i =inet_pton(AF_INET,token, &(res->nvip_addr)))!=1){ //assign IP adr myself
		printf("return: %d, inet_pton() failed @res->nvip_addr\n",i);
	}; 

	return res;
}

void print_ntableEntry(struct ntableEntry *this){

	if(this == NULL){
		printf("err @print_ntableEntry for null pointer\n");
		exit(1);		
	}
	char str1[INET_ADDRSTRLEN];
	char str2[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(this->vip_addr),str1, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(this->nvip_addr),str2, INET_ADDRSTRLEN);
	printf("virtual IP: %s --> neighbor vIP:%s\n",str1,str2);

}

void print_sockaddr (struct sockaddr_in *this){
	if(this == NULL){
		printf("err @print_sockaddr for null pointer\n");
	 	exit(1);	
	}
	char str1[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(this->sin_addr),str1, INET_ADDRSTRLEN);
	printf("IP= %s @%d\n",str1,ntohs(this->sin_port));
}


int timer_checker(time_t timer, time_t deadline){
	if ((time(NULL) - timer) >= deadline)
		return 1;
	else
		return 0;
}

int timer_reset(time_t *timer){
	time(timer);
	return 0;
	
}

int set_flag(short *flag,short value){
	*flag =value;
	return 0;
}

void print_rtableEntry(struct rtableEntry this){
	if(this.flag_exist){
		printf("-----------------------------\n");
		printf("dest: ");
		print_sockaddr(this.destination);
		printf("gate: ");
		print_sockaddr(this.gateway);
		printf("metric: %d\n", this.metric);
	}
}

int cnt_rtable(struct rtableEntry (*rtable)[]){
	int i,cnt;
	for(i = 0, cnt=0;i <MAX_RTABLE;i++){
		if((*rtable)[i].flag_exist)
			cnt++;
	}
	return cnt;
}

void print_allrtable(struct rtableEntry (*rtable)[]){
	int i;
	printf("Routing table(size:%d)\n",cnt_rtable(rtable));
	for(i=0;i<MAX_RTABLE;i++){
		print_rtableEntry((*rtable)[i]);		
	}
	printf("-----------------------------\n");

}

void print_rmsgEntry(struct rmsgEntry this){
	printf("metric: %d to dest: ",this.metric );
	print_sockaddr(&this.destination);
}

void print_rmsg(struct rmsg this){
	int i;
	// printf("-----------------------------\n");
	printf("printout rms(size:%d)\n",this.arraySize);
	for(i =0;i<this.arraySize;i++){
		print_rmsgEntry(this.array[i]);
	}
	printf("-----------------------------\n");
}



int is_sockaddr_same(struct sockaddr_in *a, struct sockaddr_in *b){
	if(a == NULL || b == NULL)
		return 0;//error

	char aip[INET_ADDRSTRLEN];
	char bip[INET_ADDRSTRLEN];
	short port_a, port_b;

	inet_ntop(AF_INET, &(a->sin_addr),aip, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(b->sin_addr),bip, INET_ADDRSTRLEN);

	port_a = ntohs(a->sin_port);
	port_b = ntohs(b->sin_port);

	if(strcmp(aip, bip) == 0 && port_a == port_b){
		// printf("cmparing: ");
		// print_sockaddr(a);
		// printf("and ");
		// print_sockaddr(b);
		return 1; //same
	}
	else
		return 0; //diff
}

int make_msg(struct rtableEntry (*rtable)[], struct rmsg *msg, struct sockaddr_in *who_to_send){
	int i,cnt;

	for(i = 0, cnt =0 ; i< MAX_RTABLE; i++){
		if((*rtable)[i].flag_exist){
			msg->array[cnt].destination = *((*rtable)[i].destination);

			if (is_sockaddr_same( (*rtable)[i].gateway, who_to_send) == 1){
				msg->array[cnt].metric = 16;			
			}
			else{
				msg->array[cnt].metric = (*rtable)[i].metric;	
			}
			
			cnt++;
		}
	}
	msg->arraySize = cnt;

	return 0;
}
int is_neighbor(struct sockaddr_in *(*narray)[],
	int narraySize, 
	struct sockaddr_in *test){

	int i;
	for(i=0;i<narraySize;i++){
		if(is_sockaddr_same(test, (*narray)[i]) == 1)
			return 1;
	}
	return 0;
}


int find_empty_slot(struct rtableEntry (*rtable)[]){
	int i;
	for(i=0;i<MAX_RTABLE;i++){
		if(  (*rtable)[i].flag_exist ==0 )
		return i; 
	}
	return -1;
}

int find_target_slot(struct rtableEntry (*rtable)[], struct sockaddr_in *target){
	int i;
	
	for(i=0;i<MAX_RTABLE;i++){
		if( (*rtable)[i].flag_exist && 
			(is_sockaddr_same((*rtable)[i].destination, target) ==1) ) 
			return i; 
	}
	return -1;
}


int find_metric(struct rtableEntry (*rtable)[] ,struct sockaddr_in *search){
	int i;

	if (search ==NULL){
		printf("err@find_metric :search null ptr\n");
		return -2; //error
	}

	for(i=0;i<MAX_RTABLE;i++){
		if( (is_sockaddr_same((*rtable)[i].destination, search) == 1) &&
			(*rtable)[i].flag_exist){
			return (*rtable)[i].metric;
		}
	}

	return -1; //not exist in the rtable
}


int is_exist_in_rtable(struct rtableEntry (*rtable)[] ,struct sockaddr_in *search){
	int i;

	if (search ==NULL){
		printf("err@is_exist_in_rtable :search null ptr\n");
		return -2; //error
	}

	for(i=0;i<MAX_RTABLE;i++){
		if( (is_sockaddr_same((*rtable)[i].destination, search) == 1) &&
			((*rtable)[i].flag_exist == 1) ){
			return i;
		}
	}
	return -1; //not exist in the rtable
}




















