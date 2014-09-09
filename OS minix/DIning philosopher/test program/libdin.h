#include <stdio.h>
#include <unistd.h> //sleep and usleep
#include <stdlib.h>
#include <libsem.h> //sem prototype
//#include <mycalllib.h> //mycall()
#include <sys/types.h>
#include <sys/wait.h> //waitpid
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>


#define N 5
#define LEFT(i) (i+N-1)%N
#define RIGHT(i) (i+1)%N
#define THINKING 0
#define HUNGRY 1
#define EATING 2

//semaphore
#define MUTEX 999

//time
#define MAX_THINK 10
#define MIN_EAT 1
#define MAX_EAT 10

//for shared memory
#define SHM_SIZE 1024
key_t key;
int shmid;
int (*state)[N+1]; //for philosopher's state. ptr to shared memory array



//fn proto type
int r_time(int min,int max); //return int range from min to max, including min and max
void think(int i);
void eat(int i);
void test(int i);
void take_forks(int i);
void put_forks(int i);
void philosopher(int i);

