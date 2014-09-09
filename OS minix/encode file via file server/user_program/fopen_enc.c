#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <lib.h>

int pwd2key(char *pwd){
	int sum = 0;
	int i;
	for(i=0;i<strlen(pwd);i++){
		sum+=*(pwd+i);
	}

	return sum % 128;
}

int sys_fopen(const char *filename, const char *pwd){ //97 callnr
//	printf("in syscall\n");
	message m;

	m.m1_i1= strlen(filename)+1;
	m.m1_p1= (char*)filename;

	m.m1_i2= strlen(pwd)+1;
	m.m1_p2= (char*)pwd;

	return (_syscall(VFS_PROC_NR, FOPEN_ENC, &m));
}

FILE *fopen_enc(char *file, char *mode, char *pwd){
	int r;
	r=sys_fopen(file,pwd);

	FILE *fp;
	fp=fopen(file,mode);

	if(r==1){
		fp->is_encoded=1;
		fp->key = pwd2key(pwd);
		fp->is_pwdright =1;
//		printf("write in key:%d",fp->key);
	}
	else if(r==-1){
		printf("Wrong password. File access denied.\n");
		exit(-1);
	}

	return fp;
}


int main()
{

	FILE *fp;

	fp=fopen_enc("t1","r","lucas");

//	printf("is_encoded:%d\n",fp->is_encoded);
//	printf("is_pwdright:%d\n",fp->is_pwdright);
//	printf("fp->key:%d\n",fp->key);

	char *cout=malloc(10);

	fread(cout,8,1,fp);

	*(cout+8)='\0';

	printf("cout:%s\n",cout);



	fp=fopen_enc("t2","w","lucas");

	strcpy(cout,"lucas");

	printf("buffer to be written:%s\n",cout);

//	printf("is_encoded:%d\n",fp->is_encoded);
//	printf("is_pwdright:%d\n",fp->is_pwdright);
//	printf("fp->key:%d\n",fp->key);


	fwrite(cout,5,1,fp);

	//	fclose(fp);


	return 0;
}
