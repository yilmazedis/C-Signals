#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h>
#include<fcntl.h>

#define MSGSIZE 1000

static char *rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

int **fillMatrixFromFile(int **matrix, int n, char* filename) {
	
	int fd;
	int i, j;
	char buffer[1024];
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


void myRead(int fd, char *buf) {

	const size_t perSize = 128;
	int count = 0;

	while(perSize == read(fd, &buf[count], perSize)) {	
		count += perSize;
	}
}


int main() 
{ 
	char inbuf[MSGSIZE]; 
	int p[2], pid, nbytes; 
	int i;

	if (pipe(p) < 0) 
		exit(1); 

	/* continued */
	if ((pid = fork()) > 0) { 
		
		rand_string(inbuf, MSGSIZE);



		fprintf(stderr, "%s\n", inbuf);

		fprintf(stderr, "\n\n" );
		
		write(p[1], inbuf, MSGSIZE);

		// Adding this line will 
		// not hang the program 
		// close(p[1]); 
		wait(NULL); 
	} 

	else { 
		sleep(1);
		int bufsize = 1024;
		int count = 0;
		// Adding this line will 
		// not hang the program 
		// close(p[1]);
		/*
		while(bufsize == read(p[0], inbuf, bufsize)) {	
			count++;
			printf("%s\n", inbuf);
		}
		count++;
		*/

		myRead(p[0], inbuf);

		fprintf(stderr, "%s\n", inbuf);
			 

		
		printf("Finished reading. Size %ld\n", strlen(inbuf)); 

		exit(0);
	} 
	return 0; 
} 
/*
void denseMultiply() {
	
	for (int i = 0; i < matrixA_row_count; ++i)
	    for (int j = 0; j < matrixB_column_count; ++j)
	        for (int k = 0; k < matrixA_column_count; ++k) {
	            matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
	        }
}*/