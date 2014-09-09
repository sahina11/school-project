#include "libdin.c"

int main(){

	/*set up shared memory
	 * the key use a file to produce a random number as key
	 * then shmget use this random number to produce shmid
	 * so you can just use any no. you want to create different
	 * shmid even though there is no file at all
	 */

	key = ftok("sth",'R');
	shmid = shmget(key, SHM_SIZE,0664 | IPC_CREAT);
	state = shmat(shmid,(void *)0,0);

	(*state)[N] = 1; //table no.

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
		for(i=0;i<N;i++){
			close_semaphore(i);
		}
		close_semaphore(MUTEX);

		//set up shared memory
		key = ftok("sth2",'R');
		shmid = shmget(key, SHM_SIZE,0664 | IPC_CREAT);
		state = shmat(shmid,(void *)0,0);

		(*state)[N]= 2; //table no.

		//create semaphores
		int i;
		for(i=0;i<N;i++){ //create 0->(N-1), N semaphores for N philosophers
			create_semaphore(i);
			down_semaphore(i); //need 0 semaphore
		}

		create_semaphore(MUTEX); //mutex sd =999

		int ccpid;
		int status;

		for(i=0;i<N;i++){
			if((ccpid = fork()) ==0){
				state = shmat(shmid,(void *)0,0);
				philosopher(i);
				break;
			}
		}
		if(ccpid != 0){
			waitpid(-1,&status,0);
		}

	}
	return 0;
}


