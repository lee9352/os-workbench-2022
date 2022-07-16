#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int main();

void hello(){
    char *p = (char *)main + 0xa + 1;//指向call指令，+1表示指向跳转目标地址e8(00000000),括号内的部分
    int32_t offset = *(int32_t *)p;//call会跳转到下一条指令+offset处，
    //取出目标地址与call下一条指令的偏移量
    assert( (char *)main + 0xf + offset == (char *)hello );
}