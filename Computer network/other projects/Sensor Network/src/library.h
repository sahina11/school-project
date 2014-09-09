#define DISJOINT 1
#define BRAIDED 2

#define AP 1
#define SENSOR 2

#define COUT 0
#define QSIZE 100
#define PATHARRAYSIZE 50


struct cordinate {
	int x;
	int y;

	struct cordinate *next;
};

struct vertex {
	int kind;
	int nameNum;
	int power;
	struct cordinate *location;
	struct queue *neighbor;

};

struct event {
	struct cordinate *location;
	int powerCons;

	struct event *next;
};

struct queue {
	int array[QSIZE]; //this is hardcode size
	int head;
	int tail;
	int fullOrEmpty; //full 1, empty -1

};