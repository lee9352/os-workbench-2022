#include "thread.h"

#define N 100000000

long sum = 0;

void Tsum() {
  for (int i = 0; i < N; i++) {
    //汇编把sum++用一条指令执行
    //asm volatile("lock add $1, %0": "+m"(sum));
    // O1和O2优化会有不同的结果
    // O1把sum取出放入寄存器，然后空循环N次，最后给寄存器+N,再放回内存
    // T2取出sum放入寄存器时，T1还没写回,导致O1的结果为N

    // O2会直接把for循环去掉，用一条add指令完成sum+N ,O2的结果为2N
    sum++;
  }
}

int main() {
  create(Tsum);
  create(Tsum);
  join();
  printf("sum = %ld\n", sum);
}
