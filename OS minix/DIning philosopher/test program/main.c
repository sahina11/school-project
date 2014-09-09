#include "libdin.c"

int main(){
	//set up shared memory
	key = ftok("sth",'R');
	shmid = shmget(key, SHM_SIZE,0664 | IPC_CREAT);
	state = shmat(shmid,(void *)0,0);

	//create semaphores
	int i;
	for(i=0;i<N;i++){ //create 0->(N-1), N semaphores for N philosophers
		create_semaphore(i);
		down_semaphore(i); //need 0 semaphore
	}

	create_semaphore(MUTEX); //mutex sd =999

	int cpid;
	int status;

	for(i=0;i<N;i++){
		if((cpid = fork()) ==0){
			state = shmat(shmid,(void *)0,0);
			philosopher(i);
			break;
		}
	}
	if(cpid != 0){
		waitpid(-1,&status,0);
	}

	return 0;
}


