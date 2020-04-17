#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h>
#include<fcntl.h>

#define MAX_CHILDREN 4

void setPipes(int pipeR[][2], int pipeW[][2], int pipeNumber);
void closePipes(int pipeR[][2], int pipeW[][2], int pipeNumber);
void parent(pid_t chldPid[MAX_CHILDREN], int pipeR[][2], int pipeW[][2], int n, char* pathA, char* pathB);
void children(int pipeR[], int pipeW[], int n);
void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option);
void CHLDhandler(int sig);
void INThandler(int sig);
int **newMatrix(int **matrix, int n);
int **fillMatrix(int **matrix, int n, char* filename);
void printMatrix(int **matrix, int n);
void freeMatrix(int **matrix, int n);
char *setCommand(char *command, int **matrix, int n, int x, int y);
void getMatrixSection(int section, int n, int* x, int* y);

char input_str[100] = "hello ";
char fixed_str[] = "forgeeks.org";

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

	char concat_str[MAX_CHILDREN][100]; 
	int i;
	pid_t wpid;
	int status = 0;
	int **matrixA, **matrixB;
	char *command;
	int x, y;


	/************************************/

	matrixA = fillMatrix(newMatrix(matrixA, n), n, pathA);
	matrixB = fillMatrix(newMatrix(matrixB, n), n, pathB);

	command = (char *) calloc((2 * n) + 1, sizeof(char));

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		getMatrixSection(i, n, &x, &y);

		command = setCommand(command, matrixA, n, x, y);
		fprintf(stderr, "%s\n", command); 

		write(pipeW[i][1], command, (2 * n) + 1);

	}
	


	/***********************************/
	fprintf(stderr, "%s\n", "sdzd");
	/*
	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		// Write input string and close writing end of first 
		// pipe. 
		write(pipeW[i][1], input_str, strlen(input_str)+1); 
	}
	*/
	// Wait All Child
	while ((wpid = wait(&status)) > 0);

	fprintf(stderr, "%s\n", "sdzd");

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		// Read string from child, print it and close 
		// reading end. 
		read(pipeR[i][0], command, (2 * n) + 1); 
		printf("Concatenated string %s\n", command); 
	}


	// Free
	freeMatrix(matrixA, n);
	freeMatrix(matrixB, n);
	free(command);
	fprintf(stderr, "%s\n", "sdzd");

}	

void children(int pipeR[], int pipeW[], int n) {

	char *command;

	command = (char *) calloc((2 * n) + 1, sizeof(char));

	// Pipe Read Part
	read(pipeR[0], command, (2 * n) + 1); 
	
	/*
	sleep(4);
	// Child Operation Part
	int k = strlen(concat_str); 
	int i; 
	for (i=0; i<strlen(fixed_str); i++) 
		concat_str[k++] = fixed_str[i]; 

	concat_str[k] = '\0'; // string ends with '\0' 
	*/
	// Pipe Write Part
	write(pipeW[1], command, (2 * n) + 1); 
	

	free(command);
	exit(0); 
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

	for (i = 0; i < n / 2; ++i)
	{
		for (j = 0; j < n / 2; ++j)
		{
			command[cmdIn++] = matrix[x + i][y + j] + 48;
			command[cmdIn++] = ' ';
		}
	}

	command[cmdIn] = '\0';

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

int **fillMatrix(int **matrix, int n, char* filename) {
	
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