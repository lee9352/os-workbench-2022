#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  int n = 2;
  // 第一次fork完毕之后有两个进程，各打印一次hello
  // 第二次fork完之后有四个进程，再各自打印一次hello
  for (int i = 0; i < n; i++) {
    fork();
    printf("Hello\n");
  }
  for (int i = 0; i < n; i++) {
    wait(NULL);
  }
}
