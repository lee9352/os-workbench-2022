#include <stdio.h>

int main(){
    //setbuf(stdout,NULL);
    // 强制printf直接输出，不缓冲
    printf("Hello");
    //当输出指向bash，stdout是一个line buffer，遇到换行符或者fflush(stdout)才输出
    //fflush(stdout);
    int *p;
    p = NULL;
    *p = 1;
}