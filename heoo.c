#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>

#define SHELL "/bin/sh"

int g_var = 0;

static int _add(int a, int b) {
    printf("_add callad, a:%d, b:%d\n", a, b);
    return a+b;
}

pid_t localopen(const char *cmdstring)
{
    pid_t pid = -1;
	printf("localopen:%s\n", cmdstring);

    if( (pid = fork()) < 0)
    {
        return pid;
    }
    else if(pid == 0) //child process.
    {
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        execl(SHELL, "sh", "-c", cmdstring, (char *) 0);
        _exit(127);
    }

    //parent, fork() return child pid in process.
    return pid;
}


int main(void) {
    int n = 1;
    printf("sizeof int:%d, pointer:%d\n", sizeof(int), sizeof(int*));
	printf("sizeof constant string:%d %d\n", sizeof("10.4.2"), sizeof('a'));
    printf("sizeof unsigned long long:%d, long:%d\n", sizeof(unsigned long long), sizeof(long));
	printf("sizeof constant string:%d %d\n", sizeof("10.4.2"), sizeof('a'));
    
    printf("one n=%d, g_var=%d\n", n, g_var);
    ++n;
    --n;
    
    g_var += 20;
    g_var -= 10;
    n = _add(1, g_var);
    printf("two n=%d, g_var=%d\n", n, g_var);
	localopen("ffplay -nodisp -i -autoexit /forlinx/sound/wo.wav");
	printf("test end\n");
    
    return 0;
}
