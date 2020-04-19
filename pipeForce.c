#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h>
#include<fcntl.h>

#define MSGSIZE 16 

char* msg1 = "hello, world #1"; 
char* msg2 = "hello, world #2"; 
char* msg3 = "hello, world #3"; 

int main() 
{ 
	char inbuf[MSGSIZE]; 
	int p[2], pid, nbytes; 
	int i;

	int n = 100000;

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
	return 0; 
} 
