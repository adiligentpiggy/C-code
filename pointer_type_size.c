#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define TCP_RECV_LOG_TAG "tcp recv buf is %s"


typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long   uint64;

typedef  char  int8;
typedef  short int16;
typedef  int   int32;
typedef  long  int64;
typedef long long int   llint;

void test_func(void)
{
    char a[] = {"hello!"};
    char buf[50] = {0};
    int pre_str_len = ARRAY_SIZE(buf);
    int ret = snprintf(buf, pre_str_len, "%s size = %d", TCP_RECV_LOG_TAG, pre_str_len);
    int arr_a_len = strlen(a);
    int i;
    for (i = 0; i < arr_a_len; i++)
    {
        ret += snprintf(&buf[ret], 50-ret, "%02X ", a[i]);
    }
    //int i = 0;
    //int a = 1;
    //int b = 2;
    //printf("%d\n", a+b);
    printf("buf is %s len:%d\r\n", buf, strlen(buf));
}

void main()
{
     printf("sizeof(uint32) sizeof(unsigned long) sizeof(llint) is %ld  %ld %ld \r\n", sizeof(uint32), sizeof(unsigned long), sizeof(llint));
	 printf("sizeof(char*) is %ld \r\n", sizeof(char*));
    //test_func();
}
