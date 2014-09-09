#include "sem.h"
#include "pm.h"
#include "mproc.h"
#include "glo.h" //who_p
#include <stdlib.h> //for free()
#include <minix/com.h> //for SUSPEND
#include "proto.h" //prototype of setreply


//support function
int sd_inuse(int sd){  //not NULL ptr and it's descriptor is the same, then sd in use
	int i;
	for(i=0;i<SEMA_PER_SET;i++){
		if((mproc[who_p].ap_sema[i]!= NULL)
				&&
				(mproc[who_p].ap_sema[i]->descriptor == sd))
			return 1;
	}

	return 0;
}

int get_free_slot(void){ //return free slot no. if none, return -1
	int i;
	for(i=0;i<SEMA_PER_SET;i++){
		if(mproc[who_p].ap_sema[i] == NULL) return i;
	}

	return -1;
}

int find_slot(int sd){ //return semaphore slot in mproc sema ptr array. or -1 if none
	int i;
	for(i=0;i<SEMA_PER_SET;i++){
		if((mproc[who_p].ap_sema[i]!= NULL)
				&&
				(mproc[who_p].ap_sema[i]->descriptor == sd))
			return i;
	}

	return -1;
}

int valid_sd(int sd){
	return (sd_inuse(sd));
}

int en_q(struct queue *p_waiting){ //1 success -1 failure
	if(p_waiting->count == MAX_Q_SIZE+1) //full no more! err -1
		return -1;

	p_waiting->q[p_waiting->last] = who_p;

	p_waiting->last++;

	if(p_waiting->last > MAX_Q_SIZE)
		p_waiting->last = 0;

	p_waiting->count++;

	return 1;
}

int de_q(struct queue *p_waiting){ //rt proc_no ; -1 failure
	if(p_waiting->count == 0) //empty! no withdraw err -1
		return -1;

	int rt = p_waiting->q[p_waiting->first];

	p_waiting->first++;

	if(p_waiting->first > MAX_Q_SIZE)
		p_waiting->first = 0;

	p_waiting->count--;

	return rt;
}

//system call fn
int do_cr_sema(void){ //rt descriptor no. -1 for failure
	int sd = m_in.m1_i1;

	//test fail cases(2):
	if(sd_inuse(sd)) return -1; //duplicate semaphore descriptor

	int slot = get_free_slot();

	mproc[who_p].ap_sema[slot] = calloc(1,sizeof(struct semaphore));//0 everywhere

	struct semaphore *p_sema;
	p_sema = mproc[who_p].ap_sema[slot];//sugar

	if(p_sema == NULL) return -1; //no more resource to allocate semaphore

	//initialize semaphore
	p_sema->value = 1;
	p_sema->descriptor = sd;
	p_sema->refcount = 1;
	p_sema->p_waiting = calloc(1,sizeof(struct queue));

	/*use calloc so I don't have to initialize first, last and count
	 * they will start as 0  */

	return p_sema->descriptor;
}

int do_up_sema(void){
	int sd = m_in.m1_i1;

	//failure test
	if(valid_sd(sd) != 1) //not in use or inherent
		return -1;

	int slot = find_slot(sd);
	struct semaphore *p_sema;
	p_sema = mproc[who_p].ap_sema[slot]; //sugar

	if(p_sema->value != 0) //binary semaphore 1->2 is not allowed
		return -1;

	//two case: sb's waiting ; nobody's watiting
	if(p_sema->p_waiting->count ==0){ // no waiting
		p_sema->value++;
		return 1;
	}
	else if(p_sema->p_waiting->count >0){ // sb's waiting, wake him up
		int wakeup;
		wakeup = de_q(p_sema->p_waiting);
		setreply(wakeup,OK);
		return 1;

	}
}

int do_down_sema(void){
	int sd = m_in.m1_i1;

	//failure test
	if(valid_sd(sd) != 1) //not in use or inherent
		return -1;

	int slot = find_slot(sd);
	struct semaphore *p_sema;
	p_sema = mproc[who_p].ap_sema[slot]; //sugar

	//go into critical session or get blocked
	if(p_sema->value == 1){ //nobody is in this critical section
		p_sema->value--;
		return 1;
	}
	else if(p_sema->value == 0){
		en_q(p_sema->p_waiting);
		return SUSPEND;
	}

}

int do_cl_sema(void){ // 0 success; -1 failure
	int sd = m_in.m1_i1;

	if(valid_sd(sd)){ //valid
		int slot = find_slot(sd);

		if(mproc[who_p].ap_sema[slot]->refcount >0){
			mproc[who_p].ap_sema[slot]->refcount--;
		}

		if(mproc[who_p].ap_sema[slot]->refcount>0){//if count -- still >0, just terminate you own asscess right and leave
			mproc[who_p].ap_sema[slot] = NULL;
			return 1;
		}
		else if(mproc[who_p].ap_sema[slot]->refcount ==0){//if count =0, free memory and NULL the ptr
			free(mproc[who_p].ap_sema[slot]);
			mproc[who_p].ap_sema[slot] = NULL;
			return 1;
		}
	}
	else //not valid
		return -1;
}

