#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t flag;

void hdl (int signo)
{
    if (signo == SIGUSR1)
    {
        fprintf(stderr,"Received SIGUSR1! %d\n", getpid());
        flag = 1;
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


void mySigsunpend(int signal);
void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option);

int main(void) {

    struct sigaction sigact;

    signalSet(&sigact, hdl, 0, 'I');
    signalSet(&sigact, 0, SIGUSR1, 'A');

    mySigsunpend(SIGUSR1);

    printf("%d\n", flag);

    return 0;
}

void signalSet(struct sigaction *sigact, void (*handler)(), int signal, char option) {

    if (option == 'I') {     
        sigact->sa_handler = handler;
        sigact->sa_flags = 0;
        sigfillset(&(sigact->sa_mask));
    } else if (option == 'A') {
        if (sigaction(signal, sigact, NULL) < 0) {
            perror("sigaction()");
            exit(EXIT_FAILURE);
        }
    }
}

void mySigsunpend(int signal) {
    
    sigset_t mask;

    sigfillset(&mask);
    sigdelset(&mask, signal);

    if (sigprocmask(SIG_SETMASK, &mask, NULL) < 0) {
        perror("sigprocmask()");
        exit(EXIT_FAILURE);
    }

    sigsuspend(&mask);
}