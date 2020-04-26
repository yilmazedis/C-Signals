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
#define MAX_SIZE 121
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

typedef struct {
	int matrix[MAX_SIZE][MAX_SIZE];
	int n;
}Matrix;

void setParentPipes(int p2cPipe[][2], int pipeNumber);
void setChildsPipes(PipeWBC *pipeWBC);
void closeParentPipes(int p2cPipe[][2], int pipeNumber);
void closeChildsPipes(PipeWBC *pipeWBC);
void parent(pid_t chldPid[MAX_CHILDREN], int p2cPipe[][2], int n, char* pathA, char* pathB);
void children(int p2cPipe[], int n, int childnum, PipeWBC *pipeWBC);
void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option);
void CHLDhandler(int sig);
void INThandler(int sig);
void fillMatrixFromFile(Matrix *matrix, int n, char* filename);
void printMatrix(Matrix *matrix, int n);
void freeMatrix(int **matrix, int n);
void setCommand(Matrix *matrixTarget, Matrix *matrixSource, int n, int x, int y);
void getMatrixSection(int section, int n, int* x, int* y);
Matrix matrixMultiplication(Matrix matrixA, Matrix matrixB, Matrix matrixTempA, Matrix matrixTempB, int n, int childnum);
void myRead(int fd, char *buf);
void conductMatrix(Matrix *matrixTarget, Matrix *matrixSource, int n, int x, int y);
void userControl(int argc, char *argv[], char *pathA, char* pathB, int *n);

int main(int argc, char *argv[]) { 
 	
 	PipeWBC pipeWBC;
 	struct sigaction sigact;
	pid_t chldPid[MAX_CHILDREN]; 
	int p2cPipe[MAX_CHILDREN][2], i, n;
	char *pathA *pathB;
	
	userControl(argc, argv, pathA, pathB, &n);

	fprintf(stderr, "%s\n", pathA);
	fprintf(stderr, "%s\n", pathB);
	fprintf(stderr, "%d\n", n);

	//return 0;

	signalSet(&sigact, CHLDhandler, 0, 'I');
    signalSet(&sigact, CHLDhandler, SIGCHLD, 'A');
    signalSet(&sigact, INThandler, SIGINT, 'A');

	setParentPipes(p2cPipe, MAX_CHILDREN);
	setChildsPipes(&pipeWBC);

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		chldPid[i] = fork();

		if (chldPid[i] == 0) {
			// !!Children pipe parameters 
			// have been have to put reverse order.
			children(p2cPipe[i], n, i, &pipeWBC);
		} else if (chldPid[i] < 0) {
			fprintf(stderr, "fork %d Failed", i); 
			return 1; 
		}
	}
	parent(chldPid, p2cPipe, n, pathA, pathB);

	closeParentPipes(p2cPipe, MAX_CHILDREN);
	closeChildsPipes(&pipeWBC);
}

void parent(pid_t chldPid[MAX_CHILDREN], int p2cPipe[][2], int n, char* pathA, char* pathB) {

	int i;
	pid_t wpid;
	int status = 0;
	int x, y;

	Matrix matrixA;
	Matrix matrixB;
	Matrix matrixC;
	Matrix command;

	fillMatrixFromFile(&matrixA, n, pathA);
	fillMatrixFromFile(&matrixB, n, pathB);

	for (i = 0; i < MAX_CHILDREN; ++i)
	{	
		getMatrixSection(i, n, &x, &y);

		setCommand(&command ,&matrixA, n, x, y);
		write(p2cPipe[i][1], &command, sizeof(Matrix));

		setCommand(&command ,&matrixB, n, x, y);
		write(p2cPipe[i][1], &command, sizeof(Matrix));
	}


	// Wait All Child
	//while ((wpid = wait(&status)) > 0);

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		read(p2cPipe[i][0], &command, sizeof(Matrix));

		getMatrixSection(i, n, &x, &y);

		conductMatrix(&matrixC, &command, n / ADDED_SPACE, x, y);
		
	}

	printMatrix(&matrixC, n);

}	

void children(int p2cPipe[], int n, int childnum, PipeWBC *pipeWBC) {

	Matrix matrixA, matrixTempA;
	Matrix matrixB, matrixTempB;
	Matrix matrixC;

	// Pipe Read matrixA Part
	read(p2cPipe[0], &matrixA, sizeof(Matrix));


	// Pipe Read matrixB Part
	read(p2cPipe[0], &matrixB, sizeof(Matrix));

	// pipe_2_4, pipe_4_2
	// pipe_2_3, pipe_3_2
	// pipe_3_5, pipe_5_3
	// pipe_4_5, pipe_5_4

	if (childnum == P2_CHILD) {
		
		write(pipeWBC->pipe_2_3[1], &matrixA, sizeof(Matrix));
		write(pipeWBC->pipe_2_4[1], &matrixB, sizeof(Matrix));

		read(pipeWBC->pipe_3_2[0], &matrixTempA, sizeof(Matrix));
		read(pipeWBC->pipe_4_2[0], &matrixTempB, sizeof(Matrix));
	
	} else if (childnum == P3_CHILD) {
		
		write(pipeWBC->pipe_3_2[1], &matrixA, sizeof(Matrix));
		write(pipeWBC->pipe_3_5[1], &matrixB, sizeof(Matrix));

		read(pipeWBC->pipe_2_3[0], &matrixTempA, sizeof(Matrix));
		read(pipeWBC->pipe_5_3[0], &matrixTempB, sizeof(Matrix));

	}else if (childnum == P4_CHILD) {
		
		write(pipeWBC->pipe_4_5[1], &matrixA, sizeof(Matrix));
		write(pipeWBC->pipe_4_2[1], &matrixB, sizeof(Matrix));

		read(pipeWBC->pipe_5_4[0], &matrixTempA, sizeof(Matrix));
		read(pipeWBC->pipe_2_4[0], &matrixTempB, sizeof(Matrix));

	}else if (childnum == P5_CHILD) {
		
		write(pipeWBC->pipe_5_4[1], &matrixA, sizeof(Matrix));
		write(pipeWBC->pipe_5_3[1], &matrixB, sizeof(Matrix));

		read(pipeWBC->pipe_4_5[0], &matrixTempA, sizeof(Matrix));
		read(pipeWBC->pipe_3_5[0], &matrixTempB, sizeof(Matrix));
	}

	matrixC = matrixMultiplication(matrixA, matrixB, matrixTempA, matrixTempB, n / ADDED_SPACE, childnum);


	// Pipe Write Part
	write(p2cPipe[1], &matrixC, sizeof(Matrix)); 
	
	exit(0); 
}

Matrix matrixMultiplication(Matrix matrixA, Matrix matrixB, Matrix matrixTempA, Matrix matrixTempB, int n, int childnum) {

	Matrix matrixC;
	int i, j, k;


	for (i = 0; i < n; ++i)
	    for (j = 0; j < n; ++j)
	        for (k = 0; k < n; ++k) {
	        	if (childnum == P2_CHILD) {

	        		matrixC.matrix[i][j] += (matrixA.matrix[i][k] * matrixB.matrix[k][j]) + 
	        		(matrixTempA.matrix[i][k] * matrixTempB.matrix[k][j]);
	        	
	        	} else if (childnum == P3_CHILD) {
	        	
	        		matrixC.matrix[i][j] += (matrixA.matrix[i][k] * matrixTempB.matrix[k][j]) + 
	        		(matrixTempA.matrix[i][k] * matrixB.matrix[k][j]);
	        	
	        	} else if (childnum == P4_CHILD) {
	        	
	        		matrixC.matrix[i][j] += (matrixA.matrix[i][k] * matrixTempB.matrix[k][j]) + 
	        		(matrixTempA.matrix[i][k] * matrixB.matrix[k][j]);
	        	
	        	} else if (childnum == P5_CHILD) {
	        	
	        		matrixC.matrix[i][j] += (matrixA.matrix[i][k] * matrixB.matrix[k][j]) + 
	        		(matrixTempA.matrix[i][k] * matrixTempB.matrix[k][j]);
	        	
	        	}
	        }

	return matrixC;
}

void conductMatrix(Matrix *matrixTarget, Matrix *matrixSource, int n, int x, int y) {

	int i, j;

	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < n; ++j)
		{
			matrixTarget->matrix[i + x][j + y] = matrixSource->matrix[i][j];
		}
	}

}

void userControl(int argc, char *argv[], char *pathA, char* pathB, int *n) {

	int opt;

	if (argc != 7)
	{
	    fprintf(stderr, "Ooooooppppppsssss! You entered wrong arguments. Usage: \n");
	    fprintf(stderr, "./program -i inputPathA -j inputPathB -n matrixSize\n"); 
	    exit(EXIT_FAILURE);
	}

	while((opt = getopt(argc, argv, ":i:o:n:")) != -1)  
	{  
	    switch(opt)  
	    {    
	        case 'i':  
	            pathA = (char *) malloc(strlen(optarg)+1);
	            strcpy(pathA, optarg);  
	            break;
	        case 'o':  
	            pathB = (char *) malloc(strlen(optarg)+1);
	            strcpy(pathB, optarg);  
	            break; 
	        case 'n':  
	            sscanf(optarg, "%d", n);
	            if (*n < 0)
	            {
	                fprintf(stderr, "You entered wrong matrixSize.\n");
	                exit(EXIT_FAILURE);                   
	            }   
	            break;  
	        case ':':
	        case '?':
	            fprintf(stderr, "Ooooooppppppsssss!\nYou entered wrong arguments. Usage: \n");
	            fprintf(stderr, "./program -i inputPathA -j inputPathB -n matrixSize\n");
	            exit(EXIT_FAILURE);
	            break;  
	    }  
	}
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

void setParentPipes(int p2cPipe[][2], int pipeNumber) {

	int i;

	for (i = 0; i < pipeNumber; ++i)
	{
		if (pipe(p2cPipe[i]) == -1) 
		{ 
			fprintf(stderr, "Pipe Read %d Failed", i); 
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

void closeParentPipes(int pipe[][2], int pipeNumber) {

	int i;

	for (i = 0; i < pipeNumber; ++i)
	{
		close(pipe[i][0]); // Read
		close(pipe[i][1]); // Write
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

void setCommand(Matrix *matrixTarget, Matrix *matrixSource, int n, int x, int y) {

	int i, j;

	for (i = 0; i < n / ADDED_SPACE; ++i)
	{
		for (j = 0; j < n / ADDED_SPACE; ++j)
		{
			matrixTarget->matrix[i][j] = matrixSource->matrix[x + i][y + j];
		}
	}
}


void fillMatrixFromFile(Matrix *matrix, int n, char* filename) {
	
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
			matrix->matrix[i][j] = (int)(*ptr);
			ptr = strtok(NULL, delim);
		}
	}
}

void printMatrix(Matrix *matrix, int n){
	int i, j;

	for(i = 0; i < n; i++)
	{
		for (j = 0; j < n; ++j)
		{
			
			fprintf(stderr, "%d ", matrix->matrix[i][j]);
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
	//fprintf(stderr, "sss\n");
	while ((pid = waitpid(-1, &status, WNOHANG)) != -1) {
		//fprintf(stderr, "Pid %d exited, status %d.\n", pid, status);
	}

}

void INThandler(int sig) {
	
	fprintf(stderr, "naber\n");

}