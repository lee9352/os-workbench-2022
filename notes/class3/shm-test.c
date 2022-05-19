#include "thread.h"

int x = 0;

void Thello(int id) {
  printf("Thread id :%d\n",id);
  usleep(id * 100000);//每个线程睡眠时间不同，所以下面的输出能够按顺序,但上面的printf就不一是按序了
  //printf("Thread id :%d",id);
  printf("Hello from thread #%c\n", "123456789ABCDEF"[x++]);
  // x是全局变量
  // 线程可以共享全局变量，那么就能看到不同线程输出不同结果
}

int main() {
  for (int i = 0; i < 10; i++) {
    create(Thello);
  }
}
