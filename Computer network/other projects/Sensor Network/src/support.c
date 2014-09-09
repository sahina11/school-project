int string2int(char *input){
	char *ptr;
	ptr = input;

	int len = strlen(input);
	int i;
	int cnt = 0;

	int output = 0;

	for(i = 0; i<len; i++ ){
		if( *ptr < '0' || *ptr >'9'){
			ptr ++;
			continue;
		}
		else {
			cnt ++;
			output = 10*output + (*ptr - '0');
			ptr ++;
		}
	}

	if (cnt == 0){
		return -1;
	}
	else 
		return output;
}


char *getLine( void ){
	char *string;
	string = malloc(1000);

	char *ptr;
	ptr = string;
	
	char now;
	
	int record = 1;
	
	while(scanf("%c", &now) > 0){ //get useful line
		if(now == '\n') {
			*ptr = '\0';
			break;
		}
		if(now == '#') {
			record = 0;
			*ptr = '\0';
		}

		if(record) strcpy(ptr,&now);

		ptr++;
	}

	return string;
}

int getLen (struct cordinate *head){
	if (head == NULL)
		return 0;
	else
		return (1 + getLen(head->next));
}

struct queue *newQ(void){
	struct queue *res;
	res = calloc(1,sizeof(struct queue));
	int k;
	for (k =0;k<QSIZE;k++){
		(res->array)[k] = -1;
	}
	res->fullOrEmpty = -1;
	return res;
}


int enqueue(struct queue *q, int input){ //err -1, nor 0
	if (q ==NULL){
		printf("try to enqueue a NULL queue pointer\n");
		return -1;
	}
	else if (q->fullOrEmpty == 1){
		printf("the queue is full\n");
		return -1;
	}
	else{
		(q->array)[q->tail] = input;
		q->tail ++;
		if (q->tail == QSIZE) q->tail = 0;
		if (q->tail == q->head) q->fullOrEmpty = 1;
		if(q->fullOrEmpty ==-1) q->fullOrEmpty = 0;
		return 0;
	}
}

int dequeue(struct queue *q){
	int res;
	if (q == NULL){
		printf("try to enqueue a NULL queue pointer\n");
		return -1;
	}
	else if(q->fullOrEmpty ==-1){
		printf("the queue is empty\n");
		return -1;
	}
	else{
		res = (q->array)[q->head];
		q->head ++;
		if (q->head == QSIZE) q->head = 0;
		if (q->tail == q->head) q->fullOrEmpty = -1;
		if (q->fullOrEmpty ==1) q->fullOrEmpty = 0;
		return res;
	}
}

int qLen(struct queue *q){
	// int cnt =0;
	// int i;
	// for(i=0;i<QSIZE;i++){
	// 	if (q->array[i] != -1) cnt++;
	// }

	// return cnt;
	if(q->fullOrEmpty ==1) return QSIZE; //new, correct version
	else if(q->fullOrEmpty ==-1) return 0;
	else{
		if(q->head < q->tail) return q->tail-q->head;
		else if(q->head > q->tail)
			return QSIZE - q->head+1 + q->tail - 1;

	}
	return -1;
}

int isNeighbor(struct vertex *n1, struct vertex *n2, int cRange){
	int dx, dy;

	dx = n1->location->x - n2->location->x;
	dy = n1->location->y - n2->location->y;

	if ( pow(dx,2)+pow(dy,2) <= pow(cRange,2))
		return 1;
	else
		return 0;
}

int isAround(struct event *eventNow, struct vertex *sensor, int sRange){
	int dx, dy;

	dx = eventNow->location->x - sensor->location->x;
	dy = eventNow->location->y - sensor->location->y;

	if ( pow(dx,2)+pow(dy,2) <= pow(sRange,2))
		return 1;
	else
		return 0;
}

int isQEqual(struct queue *q1, struct queue *q2){
	int cnt = 0;
	int i;
	for(i=0;i<QSIZE;i++){
		if(q1->array[i] == q2->array[i]){
			cnt ++;
		}
	}

	if (cnt == QSIZE) return 1;
	else return 0;
}

int isPowerful(struct vertex *node, int need){
	if (node->power >= need) return 1;
	else return 0;
}
/*print out series*/

void printCordinate(struct cordinate *head){
	if (head == NULL){
		printf("NULL cordinate pointer to be printed!\n");
	}
 	else
 		printf("(%d, %d)", head->x, head->y );
}

void printCordinateList(struct cordinate *head){
	if (head == NULL){
		printf("\n");
		return;
	}
	else{
		printCordinate(head);
		printf("\n");
		printCordinateList(head->next);
	}
}

void printEvent(struct event *head){
	if (head ==NULL)
		printf("try to print Null event\n");
	else{
		printCordinate(head->location);
		printf(":%d  ", head->powerCons);
	}
}

void printEventList(struct event *head){
	if (head == NULL){
		printf("\n");
		return;
	}
	else{
		printEvent(head);
		printEventList(head->next);
	}

}

void printQ(struct queue *q){
	int i;
	printf("len:%d ",qLen(q) );
	printf("(");
	for(i=0;i<QSIZE; i++){
		printf("%d ",(q->array)[i] );
	}
	printf(")");
	// printf("(head:%d,tail:%d,full?%d)",q->head,q->tail,q->fullOrEmpty );
}


void printVertex(struct vertex *node){
	printf("(no:%d, power:%d)",
		node->nameNum,node->power);
	printf(":");
	printCordinate(node->location);
	printf(" -> ");
	printQ(node->neighbor);
	printf("\n");
}

void printPath(struct queue *path){
	int i;
	printf("hops=%d::",qLen(path) );
	for (i = qLen(path)-1; i>=0; i--){
		printf("-> %d",path->array[i]);
	}
	printf("\n");
}

