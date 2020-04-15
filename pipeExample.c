#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h>

#define MAX_CHILDREN 4

void setPipes(int pipeR[][2], int pipeW[][2], int pipeNumber);
void parent(pid_t chldPid[MAX_CHILDREN], int pipeR[][2], int pipeW[][2]);
void children(int pipeR[], int pipeW[]);

char input_str[100];
char fixed_str[] = "forgeeks.org"; 

int main(int argc, char const *argv[]) { 
 
	pid_t chldPid[MAX_CHILDREN]; 
	int i;
	int pipeFdWrite[MAX_CHILDREN][2], pipeFdRead[MAX_CHILDREN][2]; 

	scanf("%s", input_str);

	fprintf(stderr, "%s\n", input_str);
	setPipes(pipeFdRead, pipeFdWrite, MAX_CHILDREN); 

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		chldPid[i] = fork();

		if (chldPid[i] == 0) {
			// !!cchildren pipe parameters 
			// have been have to put reverse order.
			children(pipeFdWrite[i], pipeFdRead[i]);
		} else if (chldPid[i] < 0) {
			fprintf(stderr, "fork %d Failed", i); 
			return 1; 
		}
	}
	
	parent(chldPid, pipeFdRead, pipeFdWrite);

}

void parent(pid_t chldPid[MAX_CHILDREN], int pipeR[][2], int pipeW[][2]) {

	char concat_str[MAX_CHILDREN][100]; 
	int i;
	pid_t wpid;
	int status = 0;

	for (i = 0; i < MAX_CHILDREN; ++i)
	{
		//close(pipeW[i][0]); // Close reading end of first pipe 

		// Write input string and close writing end of first 
		// pipe. 
		write(pipeW[i][1], input_str, strlen(input_str)+1); 
		//close(pipeW[i][1]); 
	}

	//fprintf(stderr, "wiat all\n");

	// Wait for child to send a string
	while ((wpid = wait(&status)) > 0);

	for (i = 0; i < MAX_CHILDREN; ++i)
	{

		//close(pipeR[i][1]); // Close writing end of second pipe 

		// Read string from child, print it and close 
		// reading end. 
		read(pipeR[i][0], concat_str[i], 100); 
		printf("Concatenated string %s\n", concat_str[i]); 
		//close(pipeR[i][0]); 
	}
}	

void children(int pipeR[], int pipeW[]) {

	// Pipe Read Part
	//close(pipeR[1]); // Close writing end of first pipe 

	// Read a string using first pipe 
	char concat_str[100]; 
	read(pipeR[0], concat_str, 100); 
	//close(pipeR[0]); 

	// Concatenate a fixed string with it 
	int k = strlen(concat_str); 
	int i; 
	for (i=0; i<strlen(fixed_str); i++) 
		concat_str[k++] = fixed_str[i]; 

	concat_str[k] = '\0'; // string ends with '\0' 
	

	// Pipe Write Part
	//close(pipeW[0]); 

	// Write concatenated string and close writing end 
	write(pipeW[1], concat_str, strlen(concat_str)+1); 
	//close(pipeW[1]); 

	exit(0); 
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