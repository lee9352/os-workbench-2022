In computer science, concurrency refers to the ability of different parts or units of a program, algorithm, or problem to be executed out-of-order or in partial order, without affecting the final outcome.

并发意味着可以乱序


把一个线程理解成一个栈，栈里面的每一帧就是一个函数(函数调用就push一个新的一帧，函数返回就pop)
那么多线程就是进程内有多个这样的栈

thread.h是一个封装好的更好用的线程创建头文件
其中的join()用于等待main函数创建的所有线程执行完毕

thread_hello.c是一个很简单的双线程例子
运行时用top观察cpu，发现该进程占用率超过100%

老师上课提了一个问题
在线程A创建一个局部变量a，然后全局变量中有一个指针p指向a
那么能否在线程B中，调用指针p来访问a

shm-test.c证明了多线程可以共享内存
stack-probe.c证明线程有独立的栈，用sort把输出排序 sort -n(按数字排序) -k 6 选第六列
在thread.h中，create函数中，可以定义一个pthread_attr变量，然后调用pthread_attr_setstacksize用来指定线程栈的大小

strace 查看系统调用，clone对应创建新线程

新问题，多线程是可以同时执行的
如果两个线程同时操作一个变量(例如同时执行x++),会导致x只加了1次


## 原子性
alipay.c
两个线程同时进入付款阶段，导致余额unsigned变成非常大
bash 循环技巧
a=0;while [**有空格** condition **有空格**];(前面不能空格)do commands;a=$[$a+1];done

sum.c
多线程对共享变量做加法求和

printf是多线程安全的？

## 顺序性
对sum.c编译优化

## 可见性
内存一致性问题
输出排序统计操作
./a.out | head -n 10000 | sort | uniq -c

满足单线程eventually memory consistency的执行，在多处理器上可能看到不同的memory顺序
write buffer，只能满足单线程内存一致性，但其实真正对内存的写操作被延迟了