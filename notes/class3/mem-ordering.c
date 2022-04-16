#include "thread.h"

int x = 0, y = 0;

atomic_int flag;
#define FLAG atomic_load(&flag)
#define FLAG_XOR(val) atomic_fetch_xor(&flag, val)
#define WAIT_FOR(cond) while (!(cond)) ;

 __attribute__((noinline))
void write_x_read_y() {
  int y_val;
  asm volatile(
    "movl $1, %0;" // x = 1
    //"mfence;"
    "movl %2, %1;" // y_val = y
    : "=m"(x), "=r"(y_val) : "m"(y)
  );
  printf("%d ", y_val);
}

 __attribute__((noinline))
void write_y_read_x() {
  int x_val;
  asm volatile(
    "movl $1, %0;" // y = 1
    //"mfence;"
    "movl %2, %1;" // x_val = x
    : "=m"(y), "=r"(x_val) : "m"(x)
  );
  printf("%d ", x_val);
}

void T1(int id) {
  while (1) {
    WAIT_FOR((FLAG & 1));
    write_x_read_y();
    FLAG_XOR(1);
  }
}

void T2() {
  while (1) {
    WAIT_FOR((FLAG & 2));
    write_y_read_x();
    FLAG_XOR(2);
  }
}

void Tsync() {
  while (1) {
    x = y = 0;
    __sync_synchronize(); // full barrier
    //usleep(1);            // + delay
    assert(FLAG == 0);
    FLAG_XOR(3);
    // T1 and T2 clear 0/1-bit, respectively
    WAIT_FOR(FLAG == 0);
    printf("\n"); fflush(stdout);
  }
}
//Tsync作为开始条件，flag_xor(3)执行完毕后，T1和T2才能执行write_read函数
//理论上不能出现00和10,因为在第一个执行的printf之前，会把另一个变量置为1，那么第二个printf就会输出1 xxx
//10还是有可能的，第一个printf取出数据放入val寄存器中，但print执行之前，切换到了另一个线程，并且另一个线程先print
//然后再执行第一个printf，就会出现10

//但是输出结果中大部分都是00，少部分10，没见到11
//原因在于flag_xor修改了内存，导致执行T1和T2的处理器核的cache line invalid
//于是movl $1, %0发生cache miss
//而写操作发生miss，就可以让读操作先执行(和体系结构的内存模型有关，x86有write buffer，会延迟写)
//从而后面的读先于前面的写执行
int main() {
  create(T1);
  create(T2);
  create(Tsync);
}
