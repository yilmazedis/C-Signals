#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h>
#include<math.h>
#include<fcntl.h>

char randomPlate(int a, int b, int c, int LxM);

int main(void){

	
	int fd, LxM = 12*13;
	int a = 0, b = 0, c = 0;
	char rnd;

	fd = open("input.txt", O_WRONLY | O_TRUNC);

	for (int i = 0; i < 3 * LxM; ++i)
	{
		rnd = randomPlate(a, b, c, LxM);

		if (rnd == 'P')
			a++;

		if (rnd == 'C')
			b++;
		
		if (rnd == 'D')
			c++;

		write(fd, &rnd, sizeof(char));
	}

	

	return 0;
}

char randomPlate(int a, int b, int c, int LxM) {

	int plates[3] = {0};
	int rnd;

	if (a == LxM) {
		plates[0] = 1;
	}

	if (b == LxM) {
		plates[1] = 1;
	}

	if (c == LxM) {
		plates[2] = 1;
	}

	do {

		rnd = rand() % 3;

	} while (plates[rnd] == 1);

	if (rnd == 0)
		return 'P';

	if (rnd == 1)
		return 'C';

	if (rnd == 2)
		return 'D';

	
}