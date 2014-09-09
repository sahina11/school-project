#include "syscall.c"

#define E 1
#define D 2
#define P 3

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
			r = tcall(argv[2],argv[3],NULL,E);
			return 0;
		}
		else if(strcmp(argv[1],"-d") == 0){
			r = tcall(argv[2],argv[3],NULL,D);
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
			r = tcall(argv[1], argv[2], argv[4], P);
			return 0;
		}
	}

	return -1;
}
