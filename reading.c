#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h>
#include<fcntl.h> 

int **newMatrix(int **matrix, int n);
int **fillMatrix(int **matrix, int n, char* filename);
void printMatrix(int **matrix, int n);
void freeMatrix(int **matrix, int n);
char *setCommand(char *command, int **matrix, int n, int x, int y);

int main(int argc, char const *argv[])
{

	int **matrixA, **matrixB;
	char *command;
	int n = 8, i;
	int x, y;

	matrixA = fillMatrix(newMatrix(matrixA, n), n, "fileA.txt");
	matrixB = fillMatrix(newMatrix(matrixB, n), n, "fileB.txt");

	printMatrix(matrixA,n);	
	printMatrix(matrixB,n);	

	/*
	x = 0;
	y = 0;
	command = setCommand(command, matrixA, n, x, y);
	fprintf(stderr, "%s\n", command);

	x = 0;
	y = n / 2;
	command = setCommand(command, matrixA, n, x, y);
	fprintf(stderr, "%s\n", command);

	x = n / 2;
	y = 0;
	command = setCommand(command, matrixA, n, x, y);
	fprintf(stderr, "%s\n", command);

	x = n / 2;
	y = n / 2;
	command = setCommand(command, matrixA, n, x, y);
	fprintf(stderr, "%s\n", command);
	*/


	if (pipe(p) < 0) 
		exit(1); 

	/* continued */
	if ((pid = fork()) > 0) { 
		
		for (i = 0; i < n; ++i)
		{
			write(p[1], msg1, MSGSIZE);
		}

		// Adding this line will 
		// not hang the program 
		// close(p[1]); 
		wait(NULL); 
	} 

	else { 

		sleep(2);
		// Adding this line will 
		// not hang the program 
		// close(p[1]);
		for (i = 0; i < n; ++i)
		{
			read(p[0], inbuf, MSGSIZE);	
			printf("%s\n", inbuf);
		}
		 
			 

		fprintf(stderr, "Length %d byte\n", (int)strlen(msg1) * n);
		fprintf(stderr, "i = %d\n", i);

		printf("Finished reading\n"); 

		exit(0);
	} 


	//printMatrix(matrixA, n);
	//printMatrix(matrixB, n);

	// Free
	freeMatrix(matrixA, n);
	freeMatrix(matrixB, n);

	return 0;
}

char *setCommand(char *command, int **matrix, int n, int x, int y) {

	int i, j, cmdIn = 0;

	command = (char *) calloc(2 * n + 1, sizeof(char));

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
			matrix[i][j] = (int)((*ptr));
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