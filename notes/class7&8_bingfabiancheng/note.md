实际中的一些复杂计算（高性能计算）需要非常长的时间

# 高性能计算
高性能计算的要求：
- 计算容易并行化，把计算分成小任务
**本质还是生产者消费者问题**


# 数据中心
CAP理论
- consistency 数据一致性
- availability 时刻可用，快速响应
- partition-tolerance 故障容错

对数据中心而言，服务器数量巨大，任何一丁点小的提升都能节省很多成本

## 协程
不需要操作系统的线程
怎么理解呢？
就是自己手动写一个代码来模拟线程切换
例如
```
void foo() {
  int i;
  for (int i = 0; i < 1000; i++) {
    printf("%d\n", i);
    co_yield();
  }
}
```
在上述代码中，co_yield就是协程的api

他的作用就是保存当前正在执行的函数的状态
然后从协程库中选一个新的协程，恢复上下文，然后执行

这本质上还是一个函数调用

x86函数调用时，调用者负责看情况保存caller saved registers(调用者保存寄存器)，如果不保存，说明里面是一些后来不会用到的值

被调用者会负责保存callee saved registers(被调用者保存寄存器)，这些寄存器会在进入被调用的函数时保存，并在ret时恢复

因为他是函数调用，所以caller saved registers已经由原函数保存好了
co_yield如果需要回到某一个协程，只需要恢复对应协程调用co_yield时候的callee saved register即可

这么多个协程都会调用co_yield，那么如何确定co_yield返回时，能返回到一个正确的协程呢？
我想应该是修改栈寄存器？
因为ret的时候，会把M[rsp] -> rip
那么把rsp修改为想要运行的协程对应的值即可吧？


协程是不能物理并行的

避免在协程中调用阻塞io，会导致该线程阻塞，其他协程也不能工作了

## GO
go 解决了协程不能使用阻塞io的问题
它会在可能引起该线程阻塞的时候，切换到另一个goroutine

每一个线程都有一个go worker，然后有数个goroutines，甚至**消灭了线程切换**

# 网页并发
单线程+事件模型
有一个事件队列，按序执行队列里面的请求
执行事件会立即返回一个结果
根据结果不同往队列里添加新的不同的请求

每个事件的执行都是不被中断的

## 事件并发
用一个队列处理各种到来的事件
基于事件的并发编程最好别用阻塞io，可以用异步io代替
使用异步io时，需要在主程序里面轮询，或者中断的方法查看哪些io已经完成

## 阻塞io
io没完成的时候，该线程会被挂起，完成后再被唤醒

## 非阻塞io
io会立即告诉线程当前进展，而不会挂起线程，于是可以在该线程里处理别的事件
主线程可以通过io的返回值判断哪些io已经执行完毕，然后去处理该部分数据




# 并发bug1：死锁(知道用锁，但没用好)
四个必要条件
- 互斥
一个资源每次只能被一个进程使用

- 请求并保持
一个进程请求资阻塞时，不释放已获得的资源

- 不可剥夺

- 循环等待
若干进程之间形成头尾相接的循环等待资源关系
线程之间存在一个环路，环路上的每个线程都持有一个资源，而这个资源又是下一个线程要申请的。

## AA型
自己等自己
```
void os_run() {
  spin_lock(&list_lock);
  spin_lock(&xxx);
  spin_unlock(&xxx); // ---------+
}                          //    |
                           //    |
void on_interrupt() {      //    |
  spin_lock(&list_lock);   // <--+
  spin_unlock(&list_lock);
}
```

中断想获取list_lock，而os必须等中断返回才能释放list_lock

**使用自旋锁的时候最好关闭中断**

AA型死锁较为容易检测，详见spinlock-xv6.c中，acquire()函数会判断该自旋锁是否已经被占用，然后报错
```
if(holding(lk))
    panic("acquire");
```

## ABBA型
A等待B，B等待A
哲学家问题，每个人都拿到了左手边的叉子

解决办法，按照固定的顺序获得锁，消除“循环等待”


## 预防办法

### 循环等待
所有线程按照一致的全局顺序获取锁，就不会循环等待了

但是有时候锁太多了，很难保证全序，就尽可能保持**偏序**，暂时还不知道什么是偏序

### 持有并等待
再加一把锁，让自己可以原子的一次性获取所有锁

### 非抢占
自己获取锁失败时，就主动把已经获得的锁释放
可能会导致**活锁**

### 互斥
避免互斥
用原子指令完成一些操作，不用锁

## 调度避免
比如银行家算法
这需要你知道全局线程需要哪些锁
并且会降低并发性
不常用

## 检查和恢复
定期检查死锁然后恢复
数据库经常用


# 并发bug2：数据竞争(直接忘记用锁了)
不同线程同时访问同一段内存，且至少一个是写

需要用互斥锁保护共享数据

time of check to time of use
经常有一段代码，先检查数据合不合法，再使用该数据
但如果没有锁保护，检查完数据后，数据可能又被其他线程修改了

# 并发bug3：忘记同步了


# 处理BUG
## 上锁顺序(死锁)检查
详情见lock-site.c
锁初始化的时候，site记录在第几行进行的初始化(作为锁的唯一标志)
上锁和解锁的时候，打印所有线程上锁和解锁的日志
观察所有线程中上锁和解锁顺序
例：在T1中，上锁顺序是x,y,z
在T2中，上锁顺序是y,x
那就有上锁顺序异常

## 数据竞争检查
不多细说了

## 动态程序分析
gcc提供了一些动态分析编译选项santizer

### AddressSanitizer
地址消毒？
gcc的时候添加选项 -fsanitize=address即可

### ThreadSanitizer
可以找数据竞争
-fsanitize=thread

### MemorySanitizer

### 自己动手实现buffer overrun检查
实现协程库时，需要自己收动创建栈buffer
可以把栈顶和底留一块缓冲区，初始化为一个特殊值
定期检查这一块缓冲区是否被修改
```
#define MAGIC 0x55555555
#define BOTTOM (STK_SZ / sizeof(u32) - 1)
struct stack { char data[STK_SZ]; };

void canary_init(struct stack *s) {
  u32 *ptr = (u32 *)s;
  // 把缓冲区置为0x55555555
  for (int i = 0; i < CANARY_SZ; i++)
    ptr[BOTTOM - i] = ptr[i] = MAGIC;
}

void canary_check(struct stack *s) {
  u32 *ptr = (u32 *)s;
  for (int i = 0; i < CANARY_SZ; i++) {
    // 检查缓冲区的值
    panic_on(ptr[BOTTOM - i] != MAGIC, "underflow");
    panic_on(ptr[i] != MAGIC, "overflow");
  }
}
```

visual studio出现烫烫烫，屯屯屯的原因
visual studio会执行下列工作
未初始化栈: 0xcccccccc
未初始化堆: 0xcdcdcdcd
对象头尾: 0xfdfdfdfd
已回收内存: 0xdddddddd

而这些数据在gb2312编码下就是那几个字

### 低配lockdep
在上锁时添加一个计时器，如果长时间没获得锁，马上能定位
```
int spin_cnt = 0;
while (xchg(&locked, 1)) {
  if (spin_cnt++ > SPIN_LIMIT) {
    printf("Too many spin @ %s:%d\n", __FILE__, __LINE__);
  }
}
```
