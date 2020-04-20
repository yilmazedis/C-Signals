#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h>
#include<fcntl.h>

#define MAX_CHILDREN 4
#define MATRIX_EDGES 4
#define ADDED_SPACE 2
#define P2_CHILD 0
#define P3_CHILD 1
#define P4_CHILD 2
#define P5_CHILD 3

typedef struct {
	int pipe_2_4[2], pipe_4_2[2];
	int pipe_2_3[2], pipe_3_2[2];
	int pipe_3_5[2], pipe_5_3[2];
	int pipe_4_5[2], pipe_5_4[2];
}PipeWBC; // pipe way between child

void setParentPipes(int pipeR[][2], int pipeW[][2], int pipeNumber);
void setChildsPipes(PipeWBC *pipeWBC);
void closeParentPipes(int pipeR[][2], int pipeW[][2], int pipeNumber);
void closeChildsPipes(PipeWBC *pipeWBC);
void parent(pid_t chldPid[MAX_CHILDREN], int pipeR[][2], int pipeW[][2], int n, char* pathA, char* pathB);
void children(int pipeR[], int pipeW[], int n, int childnum, PipeWBC *pipeWBC);
void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option);
void CHLDhandler(int sig);
void INThandler(int sig);
int **newMatrix(int **matrix, int n);
int **fillMatrixFromFile(int **matrix, int n, char* filename);
int **fillMatrixFromString(int **matrix, int n, char* str);
void printMatrix(int **matrix, int n);
void freeMatrix(int **matrix, int n);
char *setCommand(char *command, int **matrix, int n, int x, int y);
void getMatrixSection(int section, int n, int* x, int* y);
int **matrixMultiplication(int **matrixA, int **matrixB, int n);
void myRead(int fd, char *buf);

int main(int argc, char const *argv[]) { 
 	
 	struct sigaction sigact;
	pid_t chldPid[MAX_CHILDREN]; 
	int i, n = 8;
	int pipeW[MAX_CHILDREN][2], pipeR[MAX_CHILDREN][2];
	PipeWBC pipeWBC;
	char *pathA = "fileA.txt";
	char *pathB = "fileB.txt";

	signalSet(&sigact, CHLDhandler, 0, 'I');
    //signalSet(&sigact, 0, SIGCHLD, 'A');
    signalSet(&sigact, INThandler, SIGINT, 'A');

	setParentPipes(pipeR, pipeW, MAX_CHILDREN);
	setChildsPipes(&pipeWBC);

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		chldPid[i] = fork();

		if (chldPid[i] == 0) {
			// !!Children pipe parameters 
			// have been have to put reverse order.
			children(pipeW[i], pipeR[i], n, i, &pipeWBC);
		} else if (chldPid[i] < 0) {
			fprintf(stderr, "fork %d Failed", i); 
			return 1; 
		}
	}
	
	parent(chldPid, pipeR, pipeW, n, pathA, pathB);

	closeParentPipes(pipeR, pipeW, MAX_CHILDREN);
	closeChildsPipes(&pipeWBC);
}

void parent(pid_t chldPid[MAX_CHILDREN], int pipeR[][2], int pipeW[][2], int n, char* pathA, char* pathB) {

	int i;
	pid_t wpid;
	int status = 0;
	int **matrixA, **matrixB, **matrixC;
	char *command, *tempCommand;
	int x, y;

	matrixA = fillMatrixFromFile(newMatrix(matrixA, n), n, pathA);
	matrixB = fillMatrixFromFile(newMatrix(matrixB, n), n, pathB);

	command = (char *) calloc(MATRIX_EDGES * n, sizeof(char));
	tempCommand = (char *) calloc(n * n * n, sizeof(char));

	for (i = 0; i < MAX_CHILDREN; ++i)
	{	
		getMatrixSection(i, n, &x, &y);

		command = setCommand(command, matrixA, n, x, y);
		//fprintf(stderr, "A pid %d - %s\n", getpid(), command);
		write(pipeW[i][1], command, MATRIX_EDGES * n);


		command = setCommand(command, matrixB, n, x, y);
		//fprintf(stderr, "B pid %d - %s\n", getpid(), command);
		write(pipeW[i][1], command, MATRIX_EDGES * n);
	}

	// Wait All Child
	while ((wpid = wait(&status)) > 0);

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		read(pipeR[i][0], command, MATRIX_EDGES * n);
		//printf("%s\n", tempCommand); 
		sprintf(tempCommand, "%s %s", tempCommand, command);
	}

	//printf("%s\n", tempCommand); 

	matrixC = fillMatrixFromString(newMatrix(matrixC, n), n, tempCommand);

	printMatrix(matrixC, n);

	// Free
	freeMatrix(matrixA, n);
	freeMatrix(matrixB, n);
	freeMatrix(matrixC, n);


	free(command);
	free(tempCommand);

}	

void children(int pipeR[], int pipeW[], int n, int childnum, PipeWBC *pipeWBC) {

	char *commandC, *commandA, *commandB;
	char *commandTempA, *commandTempB;
	int **matrixA, **matrixB, **matrixC;

	commandA = (char *) calloc(MATRIX_EDGES * n, sizeof(char));
	commandB = (char *) calloc(MATRIX_EDGES * n, sizeof(char));
	commandC = (char *) calloc(MATRIX_EDGES * n, sizeof(char));

	commandTempA = (char *) calloc(MATRIX_EDGES * n, sizeof(char));
	commandTempB = (char *) calloc(MATRIX_EDGES * n, sizeof(char));

	// Pipe Read matrixA Part
	read(pipeR[0], commandA, MATRIX_EDGES * n);
	//fprintf(stderr, "A pid %d - %s\n", getpid(), commandA);
	matrixA = fillMatrixFromString(newMatrix(matrixA, n / ADDED_SPACE), n / ADDED_SPACE, commandA);
	//printMatrix(matrixA, n / 2);

	// Pipe Read matrixB Part
	read(pipeR[0], commandB, MATRIX_EDGES * n);
	//fprintf(stderr, "B pid %d - %s\n", getpid(), commandB);
	matrixB = fillMatrixFromString(newMatrix(matrixB, n / ADDED_SPACE), n / ADDED_SPACE, commandB);
	//printMatrix(matrixB, n / 2);


	// pipe_2_4, pipe_4_2
	// pipe_2_3, pipe_3_2
	// pipe_3_5, pipe_5_3
	// pipe_4_5, pipe_5_4

	
	if (childnum == P2_CHILD) {
		
		write(pipeWBC->pipe_2_3[1], commandA, MATRIX_EDGES * n);
		write(pipeWBC->pipe_2_4[1], commandB, MATRIX_EDGES * n);

		read(pipeWBC->pipe_3_2[0], commandTempA, MATRIX_EDGES * n);
		read(pipeWBC->pipe_4_2[0], commandTempB, MATRIX_EDGES * n);

		fprintf(stderr, "childnum %d\n", childnum);
	
	} else if (childnum == P3_CHILD) {
		
		write(pipeWBC->pipe_3_2[1], commandA, MATRIX_EDGES * n);
		write(pipeWBC->pipe_3_5[1], commandB, MATRIX_EDGES * n);

		read(pipeWBC->pipe_2_3[0], commandTempA, MATRIX_EDGES * n);
		read(pipeWBC->pipe_5_3[0], commandTempB, MATRIX_EDGES * n);

		fprintf(stderr, "childnum %d\n", childnum);

	}else if (childnum == P4_CHILD) {
		
		write(pipeWBC->pipe_4_5[1], commandA, MATRIX_EDGES * n);
		write(pipeWBC->pipe_4_2[1], commandB, MATRIX_EDGES * n);

		read(pipeWBC->pipe_5_4[0], commandTempA, MATRIX_EDGES * n);
		read(pipeWBC->pipe_2_4[0], commandTempB, MATRIX_EDGES * n);

		fprintf(stderr, "childnum %d\n", childnum);

	}else if (childnum == P5_CHILD) {
		
		write(pipeWBC->pipe_5_4[1], commandA, MATRIX_EDGES * n);
		write(pipeWBC->pipe_5_3[1], commandB, MATRIX_EDGES * n);

		read(pipeWBC->pipe_4_5[0], commandTempA, MATRIX_EDGES * n);
		read(pipeWBC->pipe_3_5[0], commandTempB, MATRIX_EDGES * n);

		fprintf(stderr, "childnum %d\n", childnum);
	}


	matrixC = matrixMultiplication(matrixA, matrixB, n / ADDED_SPACE);

	commandC = setCommand(commandC, matrixC, n, 0, 0);
	//fprintf(stderr, "C pid %d - %s\n", getpid(), commandC);

	// Pipe Write Part
	write(pipeW[1], commandC, MATRIX_EDGES * n); 

	//free
	free(commandC);
	free(commandA);
	free(commandB);
	free(commandTempA);
	free(commandTempB);
	freeMatrix(matrixA, n / ADDED_SPACE);
	freeMatrix(matrixB, n / ADDED_SPACE);
	freeMatrix(matrixC, n / ADDED_SPACE);
	
	exit(0); 
}

int **matrixMultiplication(int **matrixA, int **matrixB, int n) {

	int **matrixC;
	int i, j;

	matrixC = newMatrix(matrixC, n);

	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < n; ++j)
		{
			matrixC[i][j] = matrixA[i][j] + matrixB[i][j];
		}
	}

	return matrixC;
}

void myRead(int fd, char *buf) {

	const size_t perSize = 128;
	int count = 0;

	while(perSize == read(fd, &buf[count], perSize)) {	
		count += perSize;
	}
}

void getMatrixSection(int section, int n, int* x, int* y) {

	switch(section) {
		case 0: 
			*x = 0;
			*y = 0;
			break;
		case 1: 
			*x = 0;
			*y = n / 2;
			break;
		case 2: 
			*x = n / 2;
			*y = 0;
			break;
		case 3: 
			*x = n / 2;
			*y = n / 2;
			break;
	}
}

void setChildsPipes(PipeWBC *pipeWBC) {

	if (pipe(pipeWBC->pipe_2_4) == -1) 
	{ 
			fprintf(stderr, "Pipe_2_4 Read Failed"); 
			exit(1);
	}

	if (pipe(pipeWBC->pipe_2_3) == -1) 
	{ 
			fprintf(stderr, "Pipe_2_3 Read Failed"); 
			exit(1);
	}

	if (pipe(pipeWBC->pipe_3_5) == -1)
	{ 
			fprintf(stderr, "Pipe_3_5 Read Failed"); 
			exit(1);
	}

	if (pipe(pipeWBC->pipe_4_5) == -1)
	{ 
			fprintf(stderr, "Pipe_4_5 Read Failed"); 
			exit(1);
	}

	/////

	if (pipe(pipeWBC->pipe_4_2) == -1) 
	{ 
			fprintf(stderr, "Pipe_2_4 Read Failed"); 
			exit(1);
	}

	if (pipe(pipeWBC->pipe_3_2) == -1) 
	{ 
			fprintf(stderr, "Pipe_2_3 Read Failed"); 
			exit(1);
	}

	if (pipe(pipeWBC->pipe_5_3) == -1) 
	{ 
			fprintf(stderr, "Pipe_3_5 Read Failed"); 
			exit(1);
	}

	if (pipe(pipeWBC->pipe_5_4) == -1) 
	{ 
			fprintf(stderr, "Pipe_4_5 Read Failed"); 
			exit(1);
	}
}

void setParentPipes(int pipeR[][2], int pipeW[][2], int pipeNumber) {

	int i;

	for (i = 0; i < pipeNumber; ++i)
	{
		if (pipe(pipeR[i]) == -1) 
		{ 
			fprintf(stderr, "Pipe Read %d Failed", i); 
			exit(1);
		}

		if (pipe(pipeW[i]) == -1) 
		{ 
			fprintf(stderr, "Pipe Write %d Failed", i); 
			exit(1);
		}
	}
}

void closeChildsPipes(PipeWBC *pipeWBC) {

	// Close reads
	close(pipeWBC->pipe_2_4[0]);
	close(pipeWBC->pipe_4_2[0]);
	close(pipeWBC->pipe_2_3[0]);
	close(pipeWBC->pipe_3_2[0]);
	close(pipeWBC->pipe_3_5[0]);
	close(pipeWBC->pipe_5_3[0]);
	close(pipeWBC->pipe_4_5[0]);
	close(pipeWBC->pipe_5_4[0]);

	// Close writes
	close(pipeWBC->pipe_2_4[1]);
	close(pipeWBC->pipe_4_2[1]);
	close(pipeWBC->pipe_2_3[1]);
	close(pipeWBC->pipe_3_2[1]);
	close(pipeWBC->pipe_3_5[1]);
	close(pipeWBC->pipe_5_3[1]);
	close(pipeWBC->pipe_4_5[1]);
	close(pipeWBC->pipe_5_4[1]);
}

void closeParentPipes(int pipeR[][2], int pipeW[][2], int pipeNumber) {

	int i;

	for (i = 0; i < pipeNumber; ++i)
	{
		close(pipeR[i][0]); // Read
		close(pipeR[i][1]); // Write
		close(pipeW[i][0]);
		close(pipeW[i][1]);
	}
}

void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option) {

    if (option == 'I') {     
        sigact->sa_handler = handler;
        sigact->sa_flags = 0;
        sigfillset(&(sigact->sa_mask));
    } else if (option == 'A') {

    	if (handler) {
    		sigact->sa_handler = handler;
	    }

        if (sigaction(signal, sigact, NULL) < 0) {
            perror("sigaction()");
            exit(EXIT_FAILURE);
        }
    }
}

char *setCommand(char *command, int **matrix, int n, int x, int y) {

	int i, j, cmdIn = 0;

	for (i = 0; i < n / ADDED_SPACE; ++i)
	{
		for (j = 0; j < n / ADDED_SPACE; ++j)
		{
			//command[cmdIn++] = matrix[x + i][y + j] + 48;
			command[cmdIn++] = matrix[x + i][y + j];
			command[cmdIn++] = ' ';
		}
	}

	command[--cmdIn] = '\0';

	return command;
}

int **newMatrix(int **matrix, int n) {

	int i;

	matrix = (int **) calloc(n, sizeof(int*));
	
	for (i = 0; i < n; ++i)
	{
		matrix[i] = (int *) calloc(n, sizeof(int));
	}

	return matrix;
}

int **fillMatrixFromString(int **matrix, int n, char* command) {
	
	int i, j;
	char delim[] = " ";
	char *buffer;
	char *ptr;

	buffer = (char *) calloc(MATRIX_EDGES * n * ADDED_SPACE, sizeof(char));
	
	sprintf(buffer, "%s", command);

	ptr = strtok(buffer, delim);

	for(i = 0; i < n; i++)
	{
		for (j = 0; j < n; ++j)
		{	
			//matrix[i][j] = (int)((*ptr) - '0');
			matrix[i][j] = (int)(*ptr);
			ptr = strtok(NULL, delim);
		}
	}

	free(buffer);
	return matrix;
}

int **fillMatrixFromFile(int **matrix, int n, char* filename) {
	
	int fd;
	int i, j;
	char buffer[512];
	char delim[] = " \n";

	fd = open(filename, O_RDONLY); 

	if (fd < 0) { 
		perror("not opened"); 
		exit(1);
	}

	read(fd, buffer, sizeof(buffer));

	char *ptr = strtok(buffer, delim);

	for(i = 0; i < n; i++)
	{
		for (j = 0; j < n; ++j)
		{	
			//matrix[i][j] = (int)((*ptr) - '0');
			matrix[i][j] = (int)(*ptr);
			ptr = strtok(NULL, delim);
		}
	}

	return matrix;
}

void printMatrix(int **matrix, int n) {
	int i, j;

	for(i = 0; i < n; i++)
	{
		for (j = 0; j < n; ++j)
		{
			
			fprintf(stderr, "%d ", matrix[i][j]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "\n");
}

void freeMatrix(int **matrix, int n) {

	int i;

	for (i = 0; i < n; ++i)
	{
		free(matrix[i]);
	}
	free(matrix);
}

void CHLDhandler(int sig) {
	
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) != -1) {
		//fprintf(stderr, "Pid %d exited, status %d.\n", pid, status);
	}

}

void INThandler(int sig) {
	
	fprintf(stderr, "naber\n");

}