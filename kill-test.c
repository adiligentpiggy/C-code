#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define dzlog_info printf
#define dzlog_error printf
#define MAIN_APP_PROG_NAME "kill-test"


int g_var = 0;

static int _add(int a, int b) {
    printf("_add callad, a:%d, b:%d\n", a, b);
    return a+b;
}

void local_stop_prog(pid_t pid)
{
    if(pid > 0)
    {
        dzlog_info("localstop, quit pid:%d", pid);
        kill(pid, SIGTERM);//SIGTSTP);//);//SIGINT);
    }
}

#define  LOCAL_MAX_PID_INDEX  128
int find_pid_by_name( char* ProcName, int* foundpid)
{
    DIR             *dir;
    struct dirent   *d;
    int             pid, i;
    char            *s;
    int pnlen  =0;

    i = 0;
    foundpid[0] = 0;
    pnlen = strlen(ProcName);

    /* Open the /proc directory. */
    dir = opendir("/proc");
    if (!dir)
    {
        dzlog_error("cannot open /proc");
        return -1;
    }

    /* Walk through the directory. */
    while ((d = readdir(dir)) != NULL && i< LOCAL_MAX_PID_INDEX)
    {
        char exe [PATH_MAX+1];
        char path[PATH_MAX+1];
        int len;
        int namelen;

        /* See if this is a process */
        if ((pid = atoi(d->d_name)) == 0)       continue;

        snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);
        if ((len = readlink(exe, path, PATH_MAX)) < 0)
                continue;
        path[len] = '\0';

        /* Find ProcName */
        s = strrchr(path, '/');
        if(s == NULL) continue;
        s++;

        /* we don't need small name len */
        namelen = strlen(s);
        if(namelen < pnlen)     continue;

        if(!strncmp(ProcName, s, pnlen)) {
            /* to avoid subname like search proc tao but proc taolinke matched */
            if(s[pnlen] == ' ' || s[pnlen] == '\0') {
                foundpid[i++] = pid;
            }
        }
    }

    foundpid[i] = 0;
    closedir(dir);

    return  0;

}

/**get pid by name and ppid. use logic same pidof.
//  char *name [IN], //process name
//  int ppid, //it's parent process id.
//  return it's first id.
//IMPORTANT NOTE: if id <= 0, it is invalid.
*/
int get_pid_byname(char *name, int ppid)
{
    int i = 0;
    int rv = -1;
    int retid = 0;
    int pids[LOCAL_MAX_PID_INDEX];

    memset(pids, 0, sizeof(pids));
    rv = find_pid_by_name( name, pids);
    if(!rv)
    {
        for(i=0; (i < LOCAL_MAX_PID_INDEX) && (pids[i] > 0); i++)
        {
            if(pids[i] > ppid)
            {
                retid = pids[i];
                break;
            }
        }
    }

    return retid;
}

void stop_pa_host_progress()
{
    int mainProgPid = 0;

    mainProgPid = get_pid_byname(MAIN_APP_PROG_NAME, 0);
    dzlog_info("stop_pa_host_progress, pid:%d", mainProgPid);

    if(mainProgPid > 0)
    {
        local_stop_prog(mainProgPid);
    }
}

int main(void) {
    int n = 1;
    

    printf("two n=%d, g_var=%d\n", n, g_var);
    stop_pa_host_progress();
	printf("Cannot show");
    return 0;
}
