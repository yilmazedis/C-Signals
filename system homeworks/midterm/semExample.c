#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_POINTS 20
#define NUM_COMMANDS 2

int main()
{
    double xvals[NUM_POINTS] = {1.0, 2.0, 3.0, 4.0, -2.0, 1.0, 2.0, 5.0, 6.0, 7.0, 1.0, 1.0, 3.0, 1.0, 5.0, 1.0, 2.0, 9.0, 4.0, 2.0};
    double yvals[NUM_POINTS] = {5.0 ,3.0, 1.0, 3.0, 5.0, -1.0, 2.0, 2.0, 4.0, 5.0, 1.0, 2.0, 2.0, 4.0, 2.0, 1.0, 2.0, 7.0, 4.0, 7.0};
    
    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");

	fprintf(gnuplotPipe, "plot '-' \n");

	for (int i = 0; i < NUM_POINTS; i++)
	{
	  fprintf(gnuplotPipe, "%lf %lf\n", xvals[i], yvals[i]);
	}

	fprintf(gnuplotPipe, "e");

    return 0;
}