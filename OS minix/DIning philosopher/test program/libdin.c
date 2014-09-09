#include "libdin.h"



int r_time(int min,int max){
	int i;
	srand(time(NULL));
	i = rand()%(max - min +1) + min;

	return i;
}

void think(int i){
	int time = r_time(0,MAX_THINK);
	printf("Table #%d.Philosopher #%d start to think for %d seconds\n",(*state)[N],i,time);
	fflush(stdout);
	sleep(time);
}
void eat(int i){
	int time = r_time(MIN_EAT,MAX_EAT);
	printf("Table #%d.Philosopher #%d will eat for %d seconds\n",(*state)[N],i,time);
	fflush(stdout);
	sleep(time);
}

void test(int i){
	if( (*state)[i]==HUNGRY && (*state)[LEFT(i)] != EATING && (*state)[RIGHT(i)] != EATING){
		(*state)[i] = EATING;
		up_semaphore(i);
	}
}

void take_forks(int i){
	down_semaphore(MUTEX);
	(*state)[i]= HUNGRY;
	printf("Table #%d,Philosopher #%d is hungry\n",(*state)[N],i);
	fflush(stdout);
	test(i);
	up_semaphore(MUTEX);

	down_semaphore(i); //for philosopher
}

void put_forks(int i){
	down_semaphore(MUTEX);
	(*state)[i]=THINKING;
	test(LEFT(i));
	test(RIGHT(i));
	up_semaphore(MUTEX);
}


void philosopher(int i){
	while(1){
		think(i);
		take_forks(i);
		eat(i);
		put_forks(i);
	}
}

