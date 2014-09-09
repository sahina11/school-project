#include <unistd.h>
#include <lib.h>

int create_semaphore(int sd){ //return sd no is succeed. -1 for fail
	message m;
	m.m1_i1 = sd;

	return(_syscall (PM_PROC_NR, CR_SEMA, &m)); //44 in callnr.h
}

int up_semaphore (int sd){
	message m;
	m.m1_i1 = sd;

	return(_syscall (PM_PROC_NR, UP_SEMA , &m)); //58
}

int down_semaphore (int sd){
	message m;
	m.m1_i1 = sd;

	return(_syscall (PM_PROC_NR, DOWN_SEMA, &m)); //69
}


int close_semaphore (int sd){
	message m;
	m.m1_i1 = sd;

	return(_syscall (PM_PROC_NR, CL_SEMA, &m)); //70
}
