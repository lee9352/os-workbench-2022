#include <stdio.h>

void func(){
    printf("no para\n");
}
void func1(int x){
    printf("x=%d\n",x);
}
int main(){
    void (*func_p)(int); //指定一个函数指针，接受一个int参数
    func_p=func;//指针指向了一个不接收参数的函数
    func_p(0);//可以正常执行
    func_p=func1;
    func_p(1);


}