#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>

volatile sig_atomic_t flag;

void hdl (int signo)
{
    if (signo == SIGUSR1)
    {
        fprintf(stderr,"Received SIGUSR1! %d\n", getpid());
    }
    else if (signo == SIGUSR2)
    {
        fprintf(stderr,"Received SIGUSR2! %d\n", getpid());
    }
    else if (signo == SIGTERM)
    {
        fprintf(stderr,"Received SIGTERM! %d\n", getpid());
    }

}

void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option);

int main(void) {

    struct sigaction sigact;

    signalSet(&sigact, hdl, 0, 'I');
    signalSet(&sigact, 0, SIGTERM, 'E');
    signalSet(&sigact, 0, SIGUSR2, 'E');
    signalSet(&sigact, 0, SIGUSR1, 'E');

    while(1);

    return 0;
}

void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option) {

    if (option == 'I') {     
        sigact->sa_handler = handler;
        sigact->sa_flags = 0;
        sigfillset(&(sigact->sa_mask));
    } else if (option == 'E') {
        if (sigaction(signal, sigact, NULL) < 0) {
            perror("sigaction()");
            exit(EXIT_FAILURE);
        }
    }
}