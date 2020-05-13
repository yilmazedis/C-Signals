#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>

#define MAX 100

typedef struct{
	sem_t sem;
	int C, P, D, T;
} MySharedMem;

void supplier(MySharedMem *kitchen, int LxM, int K, char *filePath);
void cooks(int id, MySharedMem *kitchen, MySharedMem *counter, int LxM, int S, int *plateNumber);
void student(int id, MySharedMem *counter, MySharedMem *table, int L, int T, int *remainStudent);
void setMemSem(MySharedMem **shared, int *id, int value);
void setMem(int **value, int *id);
int toEstimate(int a1, int b1, int c1, int a2, int b2, int c2);
void indicatePlate(int id, char plate[]);
void checkUsage(int argc, char *argv[], int *N, int *M, int *T, int *S, int *L, char **filePath);
void graphicPlot(int list[], char title[], int size);

int main(int argc, char *argv[]){

	MySharedMem *kitchen, *counter, *table;
	int kitId, couId, tabId, plaId, remId;
	pid_t cooksPid[MAX], studentPid[MAX], supplierPid, pid; 
	int i, N, M, T, S, L, K;
	int *plateNumber = 0, *remainStudent = 0, status;
	char *filePath;
	int n = 0, plotc = 0;
	

	checkUsage(argc, argv, &N, &M, &T, &S, &L, &filePath);

	int *splot, *cplot, *kplot;

	splot = (int *) calloc(3 * L * M, sizeof(int));
	cplot = (int *) calloc(3 * L * M, sizeof(int));
	kplot = (int *) calloc(3 * L * M, sizeof(int));

	K = 2*L*M + 1;

	setMemSem(&kitchen, &kitId, 1);
	setMemSem(&counter, &couId, 1);
	setMemSem(&table, &tabId, 1);
	setMem(&plateNumber, &plaId);
	setMem(&remainStudent, &remId);

	*remainStudent = M;

	for (i = 0; i < N; ++i) {
		cooksPid[i] = fork();

		if (cooksPid[i] == 0) {
			cooks(i, kitchen, counter, L * M, S, plateNumber);
		} else if (cooksPid[i] < 0) {
			fprintf(stderr, "fork %d Failed", i); 
			return 1; 
		}
	}

	for (i = 0; i < M; ++i) {
		studentPid[i] = fork();

		if (studentPid[i] == 0) {
			student(i, counter, table, L, T, remainStudent);
		} else if (studentPid[i] < 0) {
			fprintf(stderr, "fork %d Failed", i); 
			return 1; 
		}
	}
	
	supplierPid = fork();

	if (supplierPid == 0) {
		supplier(kitchen, L * M, K, filePath);
	} else if (supplierPid < 0) {
		fprintf(stderr, "fork %d Failed", i); 
		return 1; 
	}
	
	n = *plateNumber;
	while (*plateNumber < 3 * L * M) {

		
		while (n == *plateNumber);

		n = *plateNumber;

		splot[plotc] = *remainStudent - table->T;
		cplot[plotc] = counter->P + counter->C + counter->D;
		kplot[plotc] = kitchen->P + kitchen->C + kitchen->D;

		plotc++;

	}
	
	while ((pid = waitpid(-1, &status, WNOHANG)) != -1);

	graphicPlot(splot, "# of students waiting at the counter", 3 * L * M);
	graphicPlot(cplot, "# of plates at the counter", 3 * L * M);
	graphicPlot(kplot, "# of plates at the kitchen", 3 * L * M);


	// memory release
	free(filePath);
	free(splot);
	free(cplot);
	free(kplot);

	shmdt(kitchen);
	shmdt(counter);
	shmdt(table);
	shmdt(plateNumber);
	shmdt(remainStudent);

	shmctl(kitId, IPC_RMID, NULL);
	shmctl(couId, IPC_RMID, NULL);
	shmctl(tabId, IPC_RMID, NULL);
	shmctl(plaId, IPC_RMID, NULL);
	shmctl(remId, IPC_RMID, NULL);

	return 0;
}

void supplier(MySharedMem *kitchen, int LxM, int K, char *filePath) {

	int i = 0, rnd, sum;
	int fd;
	int P = 0, C = 0, D = 0;
	char plateName[11], plate;


	fd = open(filePath, O_RDONLY);

	while (i < 3*LxM)
	{
		sem_wait(&kitchen->sem);
		if (K >= (kitchen->P + kitchen->C + kitchen->D)) {
			read(fd, &plate, sizeof(char));
			
			indicatePlate(rnd, plateName);

			sum = kitchen->P + kitchen->C + kitchen->D;
			fprintf(stderr, "The supplier is going to the kitchen to deliver %s kitchen items P:%d C:%d D:%d=%d\n", plateName, kitchen->P, kitchen->C, kitchen->D, sum);			
			

			if (plate == 'P') {
				kitchen->P += 1;
				P++;
			}
			else if (plate == 'C') {
				kitchen->C += 1;
				C++;
			}
			else if (plate == 'D'){
				kitchen->D += 1;
				D++;
			}
			sum = kitchen->P + kitchen->C + kitchen->D;
			fprintf(stderr, "The supplier delivered %s – after delivery: kitchen items P:%d C:%d D:%d=%d\n", plateName, kitchen->P, kitchen->C, kitchen->D, sum);
			i++;
		}	
		sem_post(&kitchen->sem);
	}
	
	fprintf(stderr, "The supplier finished supplying – GOODBYE!\n");

	close(fd);

	exit(0);
}

void cooks(int id, MySharedMem *kitchen, MySharedMem *counter, int LxM, int S, int *plateNumber) {

	int flag = 0;
	int count = 0, sum;

	while(*plateNumber < 3*LxM) {

		sem_wait(&kitchen->sem);
		sem_wait(&counter->sem);
		
		if (kitchen->P > 0 || kitchen->C > 0 || kitchen->D > 0 ) {

			if (S >= (counter->P + counter->C + counter->D)) {

				sum = kitchen->P + kitchen->C + kitchen->D;
				fprintf(stderr, "Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d C:%d D:%d\n", id, kitchen->P, kitchen->C, kitchen->D);
			

				switch(toEstimate(kitchen->P, kitchen->C, kitchen->D,
								  counter->P, counter->C, counter->D)) {

					case 0:
						kitchen->P -= 1;
						
						*plateNumber += 1;

						fprintf(stderr, "Cook %d is going to the counter to deliver soup – counter items P:%d C:%d D:%d\n", id, counter->P, counter->C, counter->D);
						counter->P += 1;
						fprintf(stderr, "Cook %d placed soup on the counter – counter items P:%d C:%d D:%d\n", id, counter->P, counter->C, counter->D);
						break;
					
					case 1:
						kitchen->C -= 1;
						
						*plateNumber += 1;
												
						fprintf(stderr, "Cook %d is going to the counter to deliver main course – counter items P:%d C:%d D:%d\n", id, counter->P, counter->C, counter->D);
						counter->C += 1;
						fprintf(stderr, "Cook %d placed main course on the counter – counter items P:%d C:%d D:%d\n", id, counter->P, counter->C, counter->D);
						break;
					
					case 2:
						kitchen->D -= 1;
						
						*plateNumber += 1;

						fprintf(stderr, "Cook %d is going to the counter to deliver desert – counter items P:%d C:%d D:%d\n", id, counter->P, counter->C, counter->D);
						counter->D += 1;
						fprintf(stderr, "Cook %d placed desert on the counter – counter items P:%d C:%d D:%d\n", id, counter->P, counter->C, counter->D);
						
						break;
					
					case -1:
						break;

				}
			}
		}

		sem_post(&counter->sem);
		sem_post(&kitchen->sem);
	}

	sum = kitchen->P + kitchen->C + kitchen->D;
	fprintf(stderr, "Cook %d finished serving - items at kitchen: %d – going home – GOODBYE!!!\n", id, sum);

	exit(0);
}

void student(int id, MySharedMem *counter, MySharedMem *table, int L, int T, int *remainStudent) {

	int i = 0;
	int ns = 0;
	int flag  = 0;

	while (i < L)
	{	

		fprintf(stderr, "Student %d is going to the counter (round %d) - # of students at counter: %d and counter items P:%d C:%d D:%d\n", id, i, *remainStudent - table->T,counter->P, counter->C, counter->D);
		

		sem_wait(&counter->sem);

		
		if (counter->P > 0 && counter->C > 0 && counter->D > 0) {



			counter->P -= 1;
			counter->C -= 1;
			counter->D -= 1;

			fprintf(stderr, "Student %d got food and is going to get a table (round %d) - # of empty tables: 4\n", id, i);

			
			flag = 1;

			if (i == L)
				*remainStudent -= 1;
		}
		
		sem_post(&counter->sem);


		if (flag == 1) {

			// Eating Time
			sem_wait(&table->sem);
			table->T += 1;
			
			
			fprintf(stderr, "Student %d sat at table %d to eat (round %d) - empty tables: %d\n", id, table->T, i, T - table->T);
			
			++i;

			fprintf(stderr, "Student %d left table %d to eat again (round %d) - empty tables:%d\n", id, table->T, i, T - table->T);
			
			table->T -= 1;

			
			sem_post(&table->sem);

			flag = 0;
		}

	}

	fprintf(stderr, "Student %d is done eating L=%d times - going home – GOODBYE!!!\n", id, L);
	
	exit(0);
}

void graphicPlot(int list[], char title[],int size) {

	char * commandsForGnuplot[] = {"set title \"%s\"" "set style line 1 lt 10 lw 20 pt 50 ps 60", "plot 'data.temp'"};
    FILE * temp = fopen("data.temp", "w");
    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE * gnuplotPipe = popen ("gnuplot -persistent -background coral", "w");
    int i;
    for (i=0; i < size; i++)
    {
    	fprintf(temp, "%lf %lf \n", (double)i, (double)list[i]); //Write the data to a temporary file
    }

    for (i=0; i < 3; i++)
    {
    	fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
    }


	pclose(gnuplotPipe);
}

int toEstimate(int a1, int b1, int c1, int a2, int b2, int c2) {

	if (a2 == 0)
		if (a1 > 0)
			return 0;
		else
			return -1;

	if (b2 == 0)
		if (b1 > 0)
			return 1;
		else
			return -1;

	if (c2 == 0)
		if (c1 > 0)
			return 2;
		else
			return -1;

	if (a1 == 0 || b1 == 0 || c1 == 0)
		if (a1 > b1)
			if (a1 > c1)
				return 0;
			else
				return 2;
		else
			if (b1 > c1)
				return 1;
			else
				return 2;
	else
		if (a1 < b1)
			if (a1 < c1)
				return 0;
			else
				return 2;
		else
			if (b1 < c1)
				return 1;
			else
				return 2;
	return -1;
}

void indicatePlate(int id, char plate[]) {

	switch(id) {
		case 0: sprintf(plate ,"%s", "soup"); break;
		case 1: sprintf(plate ,"%s", "main course"); break;
		case 2: sprintf(plate ,"%s", "desert"); break;
	}
}

void setMemSem(MySharedMem **shared, int *id, int value) {

	*id  = shmget(IPC_PRIVATE, sizeof(MySharedMem), IPC_CREAT | 0644);
	*shared = shmat(*id, 0, 0);
	(*shared)->P = 0;
	(*shared)->C = 0;
	(*shared)->D = 0;
	(*shared)->T = 0;
	sem_init(&(*shared)->sem, 1, value);
}

void setMem(int **value, int *id) {
	
	*id  = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0644);
	*value = shmat(*id, 0, 0);
	**value = 0;	
}

void checkUsage(int argc, char *argv[], int *N, int *M, int *T, int *S, int *L, char **filePath) {
    
    int opt;
    
    if (argc != 13)
    {
        fprintf(stderr, "Ooooooppppppsssss! You entered wrong arguments. Usage: \n");
        fprintf(stderr, "./program -N # -M # -T # -S # -L # -F filePath\n");
        exit(EXIT_FAILURE);
    }
    while((opt = getopt(argc, argv, ":N:M:T:S:L:F:")) != -1)
    {
        switch(opt)
        {
            case 'N':
                sscanf(optarg, "%d", N);
                if (*N <= 2)
                {
                    fprintf(stderr, "Cook numbers less than or equal 2!\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'M':
                sscanf(optarg, "%d", M);
                if (*M <= 2)
                {
                    fprintf(stderr, "Student numbers less than or equal 2!\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'T':
                sscanf(optarg, "%d", T);
                if (*T <= 0)
                {
                    fprintf(stderr, "Table numbers less than or equal 0!\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'S':
                sscanf(optarg, "%d", S);
                if (*S <= 3)
                {
                    fprintf(stderr, "Counter size less than or equal 3!\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'L':
                sscanf(optarg, "%d", L);
                if (*L < 3)
                {
                    fprintf(stderr, "Round number less than 3!\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'F':
                *filePath = (char*)malloc(strlen(optarg)+1);
                sprintf(*filePath, "%s", optarg);
                break;
            case ':':
            case '?':
                fprintf(stderr, "Ooooooppppppsssss! You entered wrong arguments. Usage: \n");
                fprintf(stderr, "./program -N # -M # -T # -S # -L # -F filePath\n");
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (*N >= *M)
    {
        fprintf(stderr, "Cook numbers bigger than or equal to student numbers!\n");
        exit(EXIT_FAILURE);
    }
    if (*T >= *M)
    {
        fprintf(stderr, "Table numbers bigger than or equal to student numbers!\n");
        exit(EXIT_FAILURE);
    }
}