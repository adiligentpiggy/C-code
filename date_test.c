#include <time.h>
#include <stdio.h>


int main() {
	time_t now;
    struct tm * p_time = NULL;

    time(&now);
    p_time = localtime(&now);
    
    printf("%d %d %d %d:%d:%d\n",
        p_time->tm_year,
        p_time->tm_mon,
        p_time->tm_mday,
        p_time->tm_hour,
        p_time->tm_min,
        p_time->tm_sec);
        
   char tmpBuf[100] = {0};
   strftime(tmpBuf, 100, "%Y-%m-%d  %H:%M:%S", localtime(&now)); //format date and time.   
   printf("%s\n", tmpBuf);  
   
   return 0;
}