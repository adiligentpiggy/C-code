#include <stdio.h>  
  
int main()  
{  
        setenv("MALLOC_TRACE", "taoge.log", "1");  
        mtrace();  
  
        int *p = (int *)malloc(2 * sizeof(int));  
  
        return 0;  
}  