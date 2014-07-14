#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h> /* sem_open(), sem_trywait().. */
#include <fcntl.h>   
#include <pthread.h>


/* Variables Globales*/ 
int *cancha1;
int *cancha2;
int *isPlayersCreated;
int *pelota;

//semaphores
sem_t *sem_Pelota;
sem_t *sem_Cancha1;
sem_t *sem_Cancha2;

/*Función de Memoria compartida*/
int * sharedResource(int cont, int *variable){ 
	key_t shmkey;
	int shmid;
	shmkey = ftok ("/dev/null", cont);       /* nombre y numero para SharedMemory Key "shmkey" */
    
    shmid = shmget (shmkey, sizeof (int), 0644 | IPC_CREAT);
    if (shmid < 0){                           /*error*/
        perror ("shmget\n");
        exit (1);
    }
    variable = (int *) shmat (shmid, NULL, 0);
    *variable = 0;	
    return variable;
}


int main(void)
{	
	
	pid_t children[10];/*Padre almacena los hijos para luego matarlos*/
	pid_t PID;
	int numeroHijos = 0;	/* Para Padre: numero de hijos creados*/
							/*Para Hijos: numero de jugador. 0-4 equipo A, 5-9 equipo B */

	sem_t *sem;		/*      synch semaphore         *//*shared */

	int *p;			/*      shared variable         *//*shared */	

	/*Crea los recursos compartidos*/
    isPlayersCreated = sharedResource(5, isPlayersCreated);
    cancha1 = sharedResource(6, cancha1);
    cancha2 = sharedResource(7,cancha2);
    pelota = sharedResource(8,pelota);    

    

    /*semaphores*/
    sem_Pelota = sem_open ("pSem", O_CREAT | O_EXCL, 0644, 1); 
    sem_Cancha1 = sem_open ("cSem", O_CREAT | O_EXCL, 0644, 1);
    sem_Cancha2 = sem_open ("dSem", O_CREAT | O_EXCL, 0644, 1);    
    sem_unlink ("pSem");  
    sem_unlink ("cSem");  
    sem_unlink ("dSem");    


    /*Crea los 10 procesos*/
	do{
		PID = fork();
		
		if(PID < 0){
			printf("error");
		}

		// Hijo
		else if(PID == 0){
			char equipo;
			if(numeroHijos<5){
				equipo = 'A';
			}else{equipo = 'B';}
		    printf("Child Process :: PID = %d, equipo = %c, jugador # = %d\n", getpid(),equipo,numeroHijos);
			*isPlayersCreated = *isPlayersCreated + 1;

			/*Espera hasta que todos los procesos hijos sean creados para comienzo de partido*/
			while(*isPlayersCreated!=10){			
				pthread_yield(); /*Evita busy-waiting. System call que causa que el proceso seda el CPU*/
			}

			/*Loop de partido*/
			while(1){
				/*Tiempo aleatorio para tratar de agarrar la pelota*/
				int ran;
	    		ran = rand() % 15;
				sleep(ran+5);			
				if(*pelota == 0){
					/*Region Critica Pelota*/
					if(sem_trywait(sem_Pelota)==-1){ //waiting for result 
						if(errno == ETIMEDOUT){
					}
					//printf("%d fallo agarrar la pelota\n", getpid());
					//break;
					}else{
						//success						
						printf("proceso %d tiene la pelota (equipo: %c)\n", getpid(), equipo);
						sleep(1);
						int intento=0;
						while(intento<3){
							if(numeroHijos<5){ /*Diferentes jugdaores agarran diferentes canchas*/
								if(sem_trywait(sem_Cancha2)==-1){ //trata de agarrar la cancha
									if(errno == ETIMEDOUT){
									}
									printf("%d fallo agarrar la cancha\n", getpid());
									intento++;
									//break;
								}else{
									//success
									intento = 3;						
									printf("%d agarra la cancha 2 y anota\n", getpid());
									*cancha2 = *cancha2 +1;	
									sleep(1);							
									printf("%d suelta la cancha.\n", getpid());
									sem_post (sem_Cancha2);	

								}
								
							}else{							
								if(sem_trywait(sem_Cancha1)==-1){ //trata de agarrar la cancha
									if(errno == ETIMEDOUT){
									}
									printf("%d fallo agarrar la cancha\n", getpid());
									intento++;
									//break;
								}else{
									//success	
									intento = 3;					
									printf("%d agarra la cancha 1 y anota\n", getpid());
									*cancha1 = *cancha1 +1;	
									sleep(1);							
									printf("%d suelta la cancha.\n", getpid());
									sem_post (sem_Cancha1);	

								}	
							}
						}

						sleep(1);	
						printf("%d suelta la pelota.\n\n", getpid());
						sem_post (sem_Pelota);	
					}					
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
			}
			
		}		
	}while(PID > 0 && numeroHijos<10);
	if(PID>0){
		//printf("Arranca el partido!\n",*isPlayersCreated);
		int timer = 300;/*300 segundos == 5 minutos*/
		while(timer!=0){
			sleep(1);
			int min = timer/60;
			int sec = timer%60;	
			if((timer%30)==0){	/*Muestra marcador y tiempo restante cada 30 segundos*/					
				printf("-----------------------\n:: Marcador\n:: Equipo A: %d Equipo B: %d\n:: Tiempo Restante: %d:%02d\n-----------------------\n\n",*cancha2,*cancha1,min,sec);
			}			        
			timer--;

		}
		
		/*Mata a los hijos*/	
		int i =0;	
		while(i<10){
			kill(children[i],SIGTERM);
			i++;
		}
		/*Marcador Final*/	
		printf("-----------------------\n:: Fin de Partido\n:: Marcador\n:: Equipo A: %d Equipo B: %d\n-----------------------\n\n",*cancha2,*cancha1);
			
	}
	return 0;
}
