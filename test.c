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
int *cancha1;
int *cancha2;
int *isPlayersCreated;

int *pelota;

//semaphores
sem_t *sem_Pelota;
sem_t *sem_Cancha1;
sem_t *sem_Cancha2;

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

int main(void)
{	
	
	time_t now;
	pid_t children[10];
	pid_t PID;
	int numeroHijos = 0;	/* Para Padre: numero de hijos creados*/
				/*Para Hijos: numero de jugador. 0-4 equipo A, 5-9 equipo B */

	sem_t *sem;                   /*      synch semaphore         *//*shared */

	int *p;                       /*      shared variable         *//*shared */	

	/*Crea los recursos compartidos*/
    isPlayersCreated = shareResource(5, isPlayersCreated);
    cancha1 = shareResource(6, cancha1);
    cancha2 = shareResource(7,cancha2);
    pelota = shareResource(8,pelota);    

    

    /*semaphores*/
    sem_Pelota = sem_open ("pSem", O_CREAT | O_EXCL, 0644, 1); 
    sem_Cancha1 = sem_open ("pSem2", O_CREAT | O_EXCL, 0644, 1);
    sem_Cancha2 = sem_open ("pSem3", O_CREAT | O_EXCL, 0644, 1);
    /* name of semaphore is "pSem", semaphore is reached using this name */
    sem_unlink ("pSem");      
    /* unlink prevents the semaphore existing forever */


    /*Crea los 10 procesos*/
	do{
		PID = fork();
		
		if(PID < 0){
			printf("error");
		}

		// childCreation was successful
		else if(PID == 0){
		char equipo;
		if(numeroHijos<5){
			equipo = 'A';
		}else{equipo = 'B';}
	        printf("Child Process :: PID = %d, equipo = %c\n", getpid(),equipo);
		*isPlayersCreated = *isPlayersCreated + 1;
		//printf("%d\n",*isPlayersCreated);

		/*Espera para comienzo de partido*/
		while(*isPlayersCreated!=10){
			//printf("%d\n",*isPlayersCreated);
			//sleep(3);
			pthread_yield();
		}
		while(1){
			int ran;
    			ran = rand() % 10;
			sleep(ran+5);			
				if(*pelota == 0){
					/*Region Critica Pelota*/
					sem_wait(sem_Pelota);
					/*aqui agregar codigo de region critica*/
					printf("proceso %d en tiene la pelota (equipo: %c)\n", getpid(), equipo);
					*pelota = 1;
					sleep(2);
					sem_post (sem_Pelota);
					printf("proceso %d Anota\n", getpid());
					if(numeroHijos<5){
						*cancha2 = *cancha2 +1;
					}else{
						*cancha1 = *cancha1 +1;
					}
					*pelota = 0;			
					
				}
				
			
			
		}
	              	
	        }

	    // Parent
	    else if(PID > 0){    	       	
			children[numeroHijos] = PID;
			numeroHijos++;
			if(numeroHijos == 10){
				*cancha1 =0;
				*cancha2 =0;
				//*isPlayersCreated = 1;
			}
			sleep(.1);
		}
	}while(PID > 0 && numeroHijos<10);
	if(PID>0){
		//printf("Arranca el partido!\n",*isPlayersCreated);
		int timer = 300;
		while(timer!=0){
		sleep(1);
		if((timer%30)==0){
			printf(":: Marcador\n:: Equipo A: %d Equipo B: %d\n:: Tiempo Restante: %d\n",*cancha2,*cancha1,timer);
		}
		timer--;
		}
		//wait(NULL);
		printf("matando hijos\n");
		int i =0;	
		while(i<10){
			kill(children[i],SIGTERM);
			i++;
		}		
	}
	return 0;
}
