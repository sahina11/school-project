#include <unistd.h>
#include <lib.h>

int promote (int childpid){
	message m;
	m.m1_i1 = childpid;

	return(_syscall (PM_PROC_NR, PROMOTE, &m));
}
