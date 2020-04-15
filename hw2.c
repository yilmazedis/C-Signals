#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string.h>
#include <signal.h>

void p2Process();
void p1Process(pid_t pid);
void signalControl(int signum);

int main() 
{
    signal(SIGUSR2,signalControl);

    pid_t pid = fork();

    if (pid < 0)
        printf("Can't create child process\n");
    else if (pid==0)
    {
        p2Process();
    }
    else 
    {
        p1Process(pid);
    }


	return 0; 
}

void p2Process() {
    
    fprintf(stderr,"Hello from Child! %d\n", getpid());

    sigset_t suspendmask;
    sigset_t blockMask;
    sigset_t oldmask;
    int ret;


    sigemptyset(&blockMask); 
    sigaddset(&blockMask, SIGINT);

    ret = sigprocmask(SIG_BLOCK, &blockMask, &oldmask);
	    // now SIGINT is blocked 
	    // (ignore control-C) 
    // now do something without interruption
    //fprintf(stderr, "betcha can't control-C me!\n"); 
    //sleep(5);

    ret = sigprocmask(SIG_UNBLOCK, &blockMask, &oldmask); 
    
    
    
}

void p1Process(pid_t pid) {
    fprintf(stderr,"Hello from Parent! %d\n", getpid()); 

    sleep(4);    
    
    kill(pid,SIGINT);
}


void signalControl(int signum)
{

    if (signum == SIGUSR2)
    {
        fprintf(stderr,"Received SIGUSR2! %d\n", getpid());
        
    }
}
