#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <lib.h>

int tcall(const char *filename, const char *pwd, const char *new_pwd, int mode){ //79 callnr
//	printf("in tcall\n");

	message m;

	m.m7_i1 = strlen(filename) +1;
	m.m7_p1 = (char*) filename;



	m.m7_i2 = strlen(pwd) +1;
	m.m7_p2 = (char*) pwd;

	if(new_pwd != NULL){
		m.m7_i3 = strlen(new_pwd) +1;
		m.m7_i4 = (int) new_pwd;

//		printf("passing new pwd length =%u\n",m.m7_i3);
//		printf("passing pwd adr =%p\n",(char *)m.m7_i4);
//		printf("passing pwd =%s\n",(char *)m.m7_i4);


	}

	m.m7_i5 = mode;

	return (_syscall(VFS_PROC_NR, TCALL, &m));
}


void setencrypt(void){ //97 callnr
	message m;

	_syscall(VFS_PROC_NR, SETENCRYPT, &m);
}

