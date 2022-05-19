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
//10还是有可能的，第一个printf已经取出数据放入val寄存器中(取到的是0，同时把另一个变量置为1)，但print执行之前，切换到了另一个线程，
//并且另一个线程一直执行到printf，(输出了1，但是这个线程对第一个变量修改,没有被第一个线程读取到)
//然后再执行第一个printf，就会出现10

//但是输出结果中大部分都是00，少部分10，没见到11
//课上的cache miss假说
//原因在于flag_xor修改了内存，导致执行T1和T2的处理器核的cache line invalid
//于是movl $1, %0发生cache miss
//而写操作发生miss，就可以让读操作先执行(和体系结构的内存模型有关，x86有write buffer，会延迟写)**这是老师的说法**
//从而后面的读先于前面的写执行

// 但我觉得，仅仅就是store先放在buffer里面，它可以先commit，但不急着修改内存，lsu会先执行读操作，空闲时再把数据写至缓存
// 好像又有新问题，虽然store没有立刻修改缓存，但应该也会把cache line置为valid,另一个线程理论上不能从本地缓存直接读数据了
//(除非只有缓存真的被修改了才修改标志位，好像BOOM就是这样的)

//-------------------------------------------------------------
// 所以我的结论是，store先执行了，但没有马上修改缓存，然后load，
// 在缓存被修改之前，另一个线程已经完成了load
// xor原子指令也确实会造成cache miss
// 但应该不是我们看到先写后读变成先读后写的原因
int main() {
  create(T1);
  create(T2);
  create(Tsync);
}
