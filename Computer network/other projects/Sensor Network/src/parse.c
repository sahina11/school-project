#include <stdio.h>
#include <string.h>


int getNumFromLine( void ){
	char *line;
	line = getLine();
	int num = string2int(line);

	free(line);

	return num;
}

int getMode(void){
	char *mode;
	mode = malloc(10);

	scanf("%s",mode);
	scanf("%s",mode);
	scanf("%s",mode);

	char c;
	while( scanf("%c",&c) > 0)	{ //consume the rest comments
		if(c == '\n') break;
	}

	if (!strcmp (mode, "disjoint"))
		return DISJOINT;
	else
		return BRAIDED;
}

struct cordinate *makeCordinate(void){
	char c;
	char str[10];
	char *ptr;
	ptr = str;

	int x, y;
	struct cordinate *res;
	res = calloc(1, sizeof(struct cordinate));

	int cnt = 0;

	scanf("%c",&c);

	while( c != '('){

		if (c =='#') return NULL; //bury stop point here
		
		if (c =='\n'){ //for very weired bug....
			cnt ++;
			if (cnt>20) return NULL;
		}
		scanf("%c",&c);
	}

	while( c < '0' || c >'9' ) { //find the first digit of x cordinate

		scanf("%c",&c);
	}

	do{ //get the all digits of x cordinate

		*ptr = c;
		ptr++;
		scanf("%c",&c);
	}while ( c >= '0' && c <= '9');

	*ptr ='\0';
	x = string2int(str);

	ptr = str;
	while( c < '0' || c >'9' ) { //find first digit of y cor

		scanf("%c",&c);
	}

	do{

		*ptr = c;
		ptr++;
		scanf("%c",&c);
	}while ( c >= '0' && c <= '9');

	*ptr ='\0';

	y = string2int(str);

	while( c != ')'){ //find the closing ")"

		scanf("%c",&c);		
	}

	res->x = x;
	res->y = y;
	
	return res;
}

struct cordinate *makeCordinateList(void){
	struct cordinate *list; 
	struct cordinate *ptr;
	struct cordinate *now;	
	
	
	if ( (now = makeCordinate()) != NULL){ //at least one AP
		list = now;
		ptr = list;
	}
	// else return NULL; //couldn't even find an AP

	while ( (now = makeCordinate()) != NULL){ //keep finding rest APs
		ptr->next = now;
		ptr = ptr->next;
	}

	// char c;
	// while( scanf("%c",&c) > 0)	{ //consume the rest comments
	// 	if(c == '\n') break;
	// }
	// no need for this now, I use "#" as stop point.

	return list;
}

struct event *makeEvent(void){
	struct event *res;
	char c;
	char str[10];
	char *ptr;
	ptr = str;

	int powerCons;

	res = calloc(1,sizeof(struct event));

	res->location = makeCordinate();
	if (res->location == NULL){
		return NULL;
	}

	scanf("%c",&c);

	while( c < '0' || c >'9' ) { 
		scanf("%c",&c);
	}

	do{ 
		*ptr = c;
		ptr++;
		scanf("%c",&c);
	}while ( c >= '0' && c <= '9');

	*ptr ='\0';
	powerCons = string2int(str);

	res->powerCons = powerCons;

	return res;
}


struct event *makeEventList(void){
	struct event *list;
	struct event *ptr;
	struct event *now;

	if ( (now = makeEvent()) != NULL){
		list = now;
		ptr = list;
	}

	while ( (now = makeEvent()) != NULL){
		ptr->next = now;
		ptr = ptr->next;
	}

	return list;
}












