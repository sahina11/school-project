#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <lib.h>

#define E 1
#define D 2
#define P 3

int sys_setencrypt(const char *filename, const char *pwd, const char *new_pwd, int mode){ //79 callnr
//	printf("in tcall\n");

	message m;

	m.m7_i1 = strlen(filename) +1;
	m.m7_p1 = (char*) filename;



	m.m7_i2 = strlen(pwd) +1;
	m.m7_p2 = (char*) pwd;

	if(new_pwd != NULL){
		m.m7_i3 = strlen(new_pwd) +1;
		m.m7_i4 = (int) new_pwd;

	}

	m.m7_i5 = mode;

	return (_syscall(VFS_PROC_NR, SETENCRYPT, &m));
}


void usage(){
	printf("Usage: setencrypt -[ed] filename password [-p new_password]\n");
	exit(0);
}



int main(int argc, char const *argv[])
{
	int r;

	if(argc <4)
		usage();

	if(*argv[1] == '-'){ //parse as: setencrypt -ed name pwd
		if(strcmp(argv[1],"-e") == 0){
			r = sys_setencrypt(argv[2],argv[3],NULL,E);
			return 0;
		}
		else if(strcmp(argv[1],"-d") == 0){
			r = sys_setencrypt(argv[2],argv[3],NULL,D);
			return 0;
		}
		else
			usage();
	}
	else{ //change pwd mode, parse as: setencrypt name pwd -p new_pwd
		if(strcmp(argv[3],"-p") != 0){
			usage();
		}
		else{
			r = sys_setencrypt(argv[1], argv[2], argv[4], P);
			return 0;
		}
	}

	return -1;
}
