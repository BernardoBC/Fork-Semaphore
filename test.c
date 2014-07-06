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

/* global variables*/ 
int *marco1;
int *marco2;
int *isPlayersCreated;
int *pelota;

//semaphores
sem_t sem_Pelota;
sem_t sem_Marco1;
sem_t sem_Marco2;

int * shareResource(key_t shmkey, int shmid, int cont, int *variable){
	shmkey = ftok ("/dev/null", cont);       /* valid directory name and a number */
    printf ("shmkey for this = %d\n", shmkey);
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
	key_t shmkey_1;                 /*      shared memory key       */
	key_t shmkey_2;                 /*      shared memory key       */
	key_t shmkey_3;                 /*      shared memory key       */
	int shmid_0;                    /*      shared memory id        */	
	int shmid_1;                    /*      shared memory id        */
	int shmid_2;                    /*      shared memory id        */
	int shmid_3;                    /*      shared memory id        */

	sem_t *sem;                   /*      synch semaphore         *//*shared */
	pid_t pid;                    /*      fork pid                */
	int *p;                       /*      shared variable         *//*shared */

	

    isPlayersCreated = shareResource(shmkey_0, shmid_0, 5, isPlayersCreated);


    /*Marco 1 recurso compartido*/
    shmkey_1 = ftok ("/dev/null", 6);       /* valid directory name and a number */
    printf ("shmkey for marco1 = %d\n", shmkey_1);
    shmid_1 = shmget (shmkey_1, sizeof (int), 0644 | IPC_CREAT);
    if (shmid_1 < 0){                           /* shared memory error check */
        perror ("shmget\n");
        exit (1);
    }
    marco1 = (int *) shmat (shmid_1, NULL, 0);
    *marco1 = 0;

    /*Marco 2 recurso compartido*/
    shmkey_2 = ftok ("/dev/null", 7);       /* valid directory name and a number */
    printf ("shmkey for marco2 = %d\n", shmkey_2);
    shmid_2 = shmget (shmkey_2, sizeof (int), 0644 | IPC_CREAT);
    if (shmid_2 < 0){                           /* shared memory error check */
        perror ("shmget\n");
        exit (1);
    }
    marco2 = (int *) shmat (shmid_2, NULL, 0);
    *marco2 = 0;

    /*Pelota recurso compartido*/
    shmkey_3 = ftok ("/dev/null", 8);       /* valid directory name and a number */
    printf ("shmkey for pelota = %d\n", shmkey_3);
    shmid_3 = shmget (shmkey_3, sizeof (int), 0644 | IPC_CREAT);
    if (shmid_3 < 0){                           /* shared memory error check */
        perror ("shmget\n");
        exit (1);
    }
    pelota = (int *) shmat (shmid_3, NULL, 0);
    *pelota = 0;

    
    

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
	        printf("Child Process :: PID = %d, OS PID = %d, Parent PID %d\n", PID, getpid(),getppid());
	        sem_wait(sem);
	        sleep(1);
	        printf ("  Child(%d) is in critical section.\n", getpid());
        	//sleep (1);
	        *isPlayersCreated=*isPlayersCreated+1;
	        //printf ("  Child(%d) new value of *isPlayersCreated=%d.\n", getpid(), *isPlayersCreated);
	        sem_post (sem);

	        printf(" Marco1 = %d\n",*marco1);
	        printf(" Marco2 = %d\n",*marco1);
	        exit(0);

			//marco1++;
			//sleep(1);
	        	
	    }

	    // Parent
	    else if(PID > 0){    	       	
			wait(NULL);
			numeroHijos++;
			*marco1 = *marco1 + 1;
			*marco2 = *marco2 + 1;
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
		//wait(NULL);
		printf("numero de Hijos %d, isPlayersCreated %d\n",numeroHijos, *isPlayersCreated);
		/* shared memory detach */
        shmdt (isPlayersCreated);
        shmctl (shmid_0, IPC_RMID, 0);
        shmctl (shmid_1, IPC_RMID, 0);

        /* cleanup semaphores */
        sem_destroy (sem);
	}


	return 0;
}