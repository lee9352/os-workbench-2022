#include <stdio.h>
#include <stdlib.h>

const char *binary;

struct frame {
  struct frame *next; // push %rbp
  void *addr;         // call f (pushed retaddr)
};

void backtrace() {
  struct frame *f;
  char cmd[1024];
  extern char end;

  asm volatile ("movq %%rbp, %0" : "=g"(f));//把rbp寄存器的值直接赋给f
  // 在栈空间内，这里有一个天然的链表结构(当前栈底rbp存放的是上一个调用者的rbp，栈底下一个元素是返回地址)，所以一次赋值，就把整个链表赋值了
  for (; f->addr < (void *)&end; f = f->next) {
    printf("%016lx  ", (long)f->addr); fflush(stdout);
    sprintf(cmd, "addr2line -e %s %p", binary, f->addr);
    system(cmd);
  }
}

void bar() {
  backtrace();
}

void foo() {
  bar();
}

int main(int argc, char *argv[]) {
  binary = argv[0];
  foo();
}
