#include "thread.h"
//余额用usinged表示
unsigned long balance = 100;

void Alipay_withdraw(int amt) {
  //如果有并发，可能会多个线程同时进入if，
  //由于余额是usigned，会把负数的补码当作非常大的正数
  //余额就变成了很大的正数
  if (balance >= amt) {
    //老师视频中会出现0，但是我在服务器上没有出现过，可能是服务器的cpu核太多了
    //这里usleep好像会造成一次线程切换
    usleep(1); // unexpected delays
    balance -= amt;
  }
}

void Talipay(int id) {
  Alipay_withdraw(100);
}

int main() {
  create(Talipay);
  create(Talipay);
  join();
  printf("balance = %lu\n", balance);
}
