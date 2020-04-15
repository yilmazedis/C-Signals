#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t flag;

void handler1(int signo) {
    printf("In handler1\n");
    flag = 1;
}

void handler2(int signo) {
    printf("In handler2\n");
    flag = 2;
}

int main(void) {
    struct sigaction sigact;
    sigact.sa_handler = handler1;
    sigact.sa_flags = 0;
    sigfillset(&sigact.sa_mask);

    if (sigaction(SIGUSR1, &sigact, NULL) < 0) {
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }

    sigact.sa_handler = handler2;

    if (sigaction(SIGUSR2, &sigact, NULL) < 0) {
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);

    if (sigprocmask(SIG_SETMASK, &mask, NULL) < 0) {
        perror("sigprocmask()");
        exit(EXIT_FAILURE);
    }

    sigsuspend(&mask);

    printf("%d\n", flag);

    return 0;
}