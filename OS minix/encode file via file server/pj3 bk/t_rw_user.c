#include "syscall.c"

//size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
//int fseek ( FILE * stream, long int offset, int origin );
//size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
//void rewind ( FILE * stream );

struct test{
	int a;
	int b;
	char pwd[65];
}entry;

int main()
{
	FILE *fp;
	int wt;
	int rd;
	struct test *out=calloc(1,sizeof(struct test));


	printf("sizeof(struct test) = %d\n",sizeof(struct test) );

	entry.a=1;
	entry.b=2;
	strcpy(entry.pwd,"icrtiou");

	char *buffer= (char*) &entry;
	int buffersize=sizeof(entry);



	fp = fopen("io","wb+");
	wt=fwrite(buffer,buffersize,1,fp);
	printf("wt= %d\n",wt);

	rewind(fp);
	// fseek(fp,0,SEEK_SET);

	rd=fread(out,sizeof(entry),1,fp);
	printf("rd= %d\n",rd);
	
	printf("a: %d\n",out->a);
	printf("b: %d\n",out->b);
	printf("pwd: %s\n",out->pwd);


	fclose(fp);


	return 0;
}
