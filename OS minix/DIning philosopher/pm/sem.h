#ifndef SEM_H
#define SEM_H

#define SEMA_PER_SET 12
#define MAX_Q_SIZE 1000

struct semaphore{
	int value;
	int descriptor;
	int refcount;
	struct queue *p_waiting;

};

struct queue{
	int q[MAX_Q_SIZE+1];
	int first;
	int last;
	int count;
};

int sd_inuse(int sd); // 1/0
int get_free_slot(void); //rt slot no. -1 fail
int find_slot(int sd); //rt slot no. -1 fail
int valid_sd(int sd);// 1/0

int en_q(struct queue *p_waiting);// will be use in down, 1 success, -1 failure
int de_q(struct queue *p_waiting);//used in up, rt proc_no, or -1 failure

int do_cr_sema(void);//rt descriptor no. -1 for failure
int do_up_sema(void);//1 success ;-1 fail
int do_down_sema(void);//1 /-1
int do_cl_sema(void);//0 success -1 failure

#endif

