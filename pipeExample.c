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

void setPipes(int pipeR[][2], int pipeW[][2], int pipeNumber);
void closePipes(int pipeR[][2], int pipeW[][2], int pipeNumber);
void parent(pid_t chldPid[MAX_CHILDREN], int pipeR[][2], int pipeW[][2], int n, char* pathA, char* pathB);
void children(int pipeR[], int pipeW[], int n);
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

int main(int argc, char const *argv[]) { 
 	
 	struct sigaction sigact;
	pid_t chldPid[MAX_CHILDREN]; 
	int i;
	int pipeW[MAX_CHILDREN][2], pipeR[MAX_CHILDREN][2];
	int n = 8;
	char *pathA = "fileA.txt";
	char *pathB = "fileB.txt";

	signalSet(&sigact, CHLDhandler, 0, 'I');
    //signalSet(&sigact, 0, SIGCHLD, 'A');
    signalSet(&sigact, INThandler, SIGINT, 'A');

	setPipes(pipeR, pipeW, MAX_CHILDREN); 

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		chldPid[i] = fork();

		if (chldPid[i] == 0) {
			// !!Children pipe parameters 
			// have been have to put reverse order.
			children(pipeW[i], pipeR[i], n);
		} else if (chldPid[i] < 0) {
			fprintf(stderr, "fork %d Failed", i); 
			return 1; 
		}
	}
	
	parent(chldPid, pipeR, pipeW, n, pathA, pathB);

	closePipes(pipeR, pipeW, MAX_CHILDREN);
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
	free(command);
	free(tempCommand);

}	

void children(int pipeR[], int pipeW[], int n) {

	char *command;
	int **matrixA, **matrixB, **matrixC;

	command = (char *) calloc(MATRIX_EDGES * n, sizeof(char));

	// Pipe Read matrixA Part
	read(pipeR[0], command, MATRIX_EDGES * n);
	//fprintf(stderr, "A pid %d - %s\n", getpid(), command);
	matrixA = fillMatrixFromString(newMatrix(matrixA, n / ADDED_SPACE), n / ADDED_SPACE, command);
	//printMatrix(matrixA, n / 2);

	// Pipe Read matrixB Part
	read(pipeR[0], command, MATRIX_EDGES * n);
	//fprintf(stderr, "B pid %d - %s\n", getpid(), command);
	matrixB = fillMatrixFromString(newMatrix(matrixB, n / ADDED_SPACE), n / ADDED_SPACE, command);
	//printMatrix(matrixB, n / 2);


	matrixC = matrixMultiplication(matrixA, matrixB, n / ADDED_SPACE);

	command = setCommand(command, matrixC, n, 0, 0);
	//fprintf(stderr, "C pid %d - %s\n", getpid(), command);

	// Pipe Write Part
	write(pipeW[1], command, MATRIX_EDGES * n); 

	//free
	free(command);
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

void setPipes(int pipeR[][2], int pipeW[][2], int pipeNumber) {

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

void closePipes(int pipeR[][2], int pipeW[][2], int pipeNumber) {

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
			command[cmdIn++] = matrix[x + i][y + j] + 48;
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
			matrix[i][j] = (int)((*ptr) - '0');
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
			matrix[i][j] = (int)((*ptr) - '0');
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