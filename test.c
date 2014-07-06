#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h> /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>   
#include <pthread.h>

/* global variables*/ 
int *marco1;
int *marco2;
int *isPlayersCreated;
int *pelota;

//semaphores
sem_t sem_Pelota;
sem_t sem_Marco1;
sem_t sem_Marco2;

int * shareResource(int cont, int *variable){
	key_t shmkey;
	int shmid;
	shmkey = ftok ("/dev/null", cont);       /* valid directory name and a number */
    
    shmid = shmget (shmkey, sizeof (int), 0644 | IPC_CREAT);
    if (shmid < 0){                           /* shared memory error check */
        perror ("shmget\n");
        exit (1);
    }
    variable = (int *) shmat (shmid, NULL, 0);
    *variable = 0;	
    return variable;
}

int main(void)
{
	time_t now;
	pid_t PID;
	int numeroHijos = 0;

	/*Memoria compartida*/
	key_t shmkey_0;                 /*      shared memory key       */
	int shmid_0;                    /*      shared memory id        */	
	int shmid_1;                    /*      shared memory id        */
	int shmid_2;                    /*      shared memory id        */
	int shmid_3;                    /*      shared memory id        */

	sem_t *sem;                   /*      synch semaphore         *//*shared */
	pid_t pid;                    /*      fork pid                */
	int *p;                       /*      shared variable         *//*shared */

	

    isPlayersCreated = shareResource(5, isPlayersCreated);
    marco1 = shareResource(6, marco1);
    marco2 = shareResource(7,marco2);
    pelota = shareResource(8,marco2);

    
    

    /*semaphores*/
    sem = sem_open ("pSem", O_CREAT | O_EXCL, 0644, 1); 
    /* name of semaphore is "pSem", semaphore is reached using this name */
    sem_unlink ("pSem");      
    /* unlink prevents the semaphore existing forever */
    /* if a crash occurs during the execution         */
    //printf ("semaphores initialized.\n\n");

	do{
		PID = fork();
		
		if(PID < 0){
			printf("error");
		}

		// childCreation was successful
		else if(PID == 0){ 
			
			//var_lcl++;
	        //var_glb++;
	        //printf("Child Process :: PID = %d, OS PID = %d, Parent PID %d\n", PID, getpid(),getppid());
	        printf("Child Process :: PID = %d\n", getpid());

	        /*Region Critica*/
	        sem_wait(sem);
	        sleep(.1);
	        //printf ("  Child(%d) is in critical section.\n", getpid());
        	//sleep (1);	        
	        //printf ("  Child(%d) new value of *isPlayersCreated=%d.\n", getpid(), *isPlayersCreated);
	        sem_post (sem);

	        //printf("	Marco1 = %d\n",*marco1);
	        //printf("	Marco2 = %d\n",*marco2);
	        while(*isPlayersCreated==0){
	        	pthread_yield();
	        }
	        printf("PID= %d no more yield\n", getpid());
			exit(0);

			//marco1++;
			//sleep(1);
	        	
	    }

	    // Parent
	    else if(PID > 0){    	       	
			//wait(NULL);
			numeroHijos++;
			*marco1 = *marco1 + 2;
			*marco2 = *marco2 + 1;
			if(numeroHijos == 10){
				*isPlayersCreated = 1;
			}
			sleep(1);
			//var_lcl = 10;
			//var_glb = 20;
			//var_glb++;
			//printf("Parent process :: Children Count = %d, PID = %d, OS PID = %d, Parent PID %d\n\n", numeroHijos++,PID, getpid(), getppid());
			//marco1++;	
			//printf("%d\n",numeroHijos);	
		}
	}while(PID > 0 && numeroHijos<10);
	//printf(" variable global valor: %d\n", var_glb);
	if(PID>0){
		
	}


	return 0;
}