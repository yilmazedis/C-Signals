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

int doParent(pid_t pid);
int doChild();
void mySigsunpend(int signal);

int condition = 1;

static void hdl (int signo)
{
    if (signo == SIGINT)
    {
        fprintf(stderr, "\nCTRL^C geldi!!!\n");
        condition = 0;
        exit(0);
    }
}


int main(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid != 0){
        doParent(pid);
    }else{
        fprintf(stderr,"asfagkan\n");
        doChild();
    }

    return 0;
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


int doParent(pid_t pid)
{

    int line = 0;
    
    while(condition)
    {
        if (line >= 99)
        {
            fprintf(stderr, "sıgsuspend PARENT \n");
            //kill(pid,SIGUSR1);

            mySigsunpend(SIGUSR1);

            condition = 1;
            exit(0);
        }

        if (line == 10)
        {
            mySigsunpend(SIGUSR1);
            fprintf(stderr, "sıgsuspend PARENT 1 second waits \n");

            sleep(1);
            kill(pid, SIGUSR2);
        }
        ++line;
        fprintf(stderr, "Parent LİNE : %d!!!\n", line);
    }
}



int doChild()
{
    fprintf(stderr, " CHİLD \n");
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set,SIGUSR2);
    struct sigaction act;
    memset (&act,0 , sizeof(act));
    act.sa_handler = &hdl;

    if (sigaction(SIGUSR2, &act, NULL) < 0 )
    {
        perror ("sigaction child ");
        exit(0);
    }

    int line = 0;

    while(condition)
    {
        if (line == 10)
        {
            sleep(1);

            kill(getppid(),SIGUSR1);

            if (sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
                perror("sigprocset()");
                exit(EXIT_FAILURE);
            }

            fprintf(stderr, "sıgsuspend CHİLD \n");
            sigsuspend(&set);
        }

        if (line >= 99)
        {
            kill(getppid(),SIGUSR1);
            break;
        }
        ++line;
        fprintf(stderr, "Child LİNE : %d!!!\n", line);
    }
}
