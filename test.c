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

/*Memoria compartida*/
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

void jugar(){
	
}

int main(void)
{
	time_t now;
	pid_t children[10];
	pid_t PID;
	int numeroHijos = 0;/* 0-4 equipo ROJO, 5-9 equipo AZUL */

	sem_t *sem;                   /*      synch semaphore         *//*shared */

	int *p;                       /*      shared variable         *//*shared */	

	/*Crea los recursos compartidos*/
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

    /*Crea los 10 procesos*/
	do{
		PID = fork();
		
		if(PID < 0){
			printf("error");
		}

		// childCreation was successful
		else if(PID == 0){ 

	        printf("Child Process :: PID = %d\n", getpid());
	        while(*isPlayersCreated==0){
	        	pthread_yield();
	        }	
	        sleep(500);
	        while(1){

	        }
	        //jugar();        
	        
	        /*Region Critica*/
	        //sem_wait(sem);
	        /*aqui agregar codigo de region critica*/
	        //sem_post (sem);
	        /*Duerme al proceso hasta que todos los procesos hayan sido creados*/	                
			//exit(0);
			//marco1++;
			//sleep(1);
	        	
	    }

	    // Parent
	    else if(PID > 0){    	       	
			//wait(NULL);
			children[numeroHijos] = PID;
			numeroHijos++;
			printf("Parent Process. Created child PID = %d\n", PID);
			*marco1 = *marco1 + 2;
			*marco2 = *marco2 + 1;
			if(numeroHijos == 10){
				*isPlayersCreated = 1;
			}
			//sleep(1);
		}
	}while(PID > 0 && numeroHijos<10);
	if(PID>0){
		printf("Arranca el partido!\n");
		sleep(10);
		int i =0;	
		while(i<10){
			kill(children[i],SIGTERM);
			i++;
		}
		
	}
	return 0;
}