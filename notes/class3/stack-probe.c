#include "thread.h"

__thread char *base, *cur; // thread-local variables
__thread int id;

// objdump to see how thread-local variables are implemented
__attribute__((noinline)) void set_cur(void *ptr) { cur = ptr; }
__attribute__((noinline)) char *get_cur()         { return cur; }

void stackoverflow(int n) {
  set_cur(&n);
  if (n % 1024 == 0) {
    int sz = base - get_cur();
    printf("Stack size of T%d >= %d KB\n", id, sz / 1024);
  }
  //这个递归会一直执行直到栈溢出
  //每次进入递归，都会有元素入栈，栈指针都会减小
  //而cur指向最新的栈顶附近
  //溢出前的最后一次输出，可得一个线程的栈空间为8MB
  stackoverflow(n + 1);
}

void Tprobe(int tid) {
  id = tid;
  base = (void *)&tid;
  //通过输出相邻两个线程的栈起始位置，相邻两个线程的base地址间隔也为8MB
  printf("thread%d,base=%p\n",id,base);
  stackoverflow(0);
}

int main() {
  setbuf(stdout, NULL);
  for (int i = 0; i < 4; i++) {
    create(Tprobe);
  }
}
