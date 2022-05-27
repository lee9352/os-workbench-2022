#include <unistd.h>
#include <stdio.h>

int main() {
  pid_t pid1 = fork(); //fork()返回后会有两个进程
  pid_t pid2 = fork(); //四个线程
  pid_t pid3 = fork(); //八个线程
  printf("Hello World from (%d, %d, %d)\n", pid1, pid2, pid3);
}
