#include <stdio.h>
#include <unistd.h>

#include <mylib.h> // in /usr/include..   ps.do not store in /usr/src/include


int main(){
	int mainpid = getpid();
	int childpid;

	printf("\n""main pid: %d\n",getpid());
	printf("shell pid: %d\n\n",getppid());

	if( (childpid = fork()) != 0){   //parent's code .fork retrun cpid to parent
		printf("I am main program. My new born child pid is: %d\n",childpid);
		printf("type 1 to promote the child, anything to exit program: ");

		int i;
		scanf("%d", &i);

		if (i == 1){
			promote(childpid);
			/*waitpid(-1,&exitstatus,0); //actually I don't need to wait. I can
			 * die right after promotion, however succeed or not
			 */
		}


	}
	else{  //child's code
		printf("Hi, I am the child process.\n");
		printf("This is my pid: %d\n",getpid());
		printf("This is my parent's pid: %d\n",getppid());

		while(getppid() == mainpid)
			;
			/* the main program is going to die, I have to survive until got
			 * promotion. So in the condition, when he is not yet dead, I
			 * will live to see if I got promoted.
			 * In both cases either I got the promotion or he do nothing but die
			 * my ppid will not be mainpid. If I got promoted, ppid will be
			 * shell's pid, if no promote, it will be 1, then I will get out
			 * of infinite loop
			 */


		if (getppid() == 1)
			printf("I am a zombie... this is my parent's pid: %d\n\n",getppid());
		else
			printf("This is my parent's pid after promotion: %d\n\n",getppid());


	}

	return 0;
}
