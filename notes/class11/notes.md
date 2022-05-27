# 操作系统启动
上文提到，CPU启动会先执行firmware
然后执行bootloader
然后才执行内核

内核准备完毕后，会执行第一个用户进程init
之后所有的进程都由init通过系统调用创建
init不能返回

# 三类主要系统调用 
- 进程
fork,execve,exit
- 内存
mmap
- 文件
open,close,read,write
mkdir

## 其余的系统调用
网络等

# 进程调用

### 创建进程
**fork()**
叉子，分叉
指原本一个执行流，fork()之后会分叉

把调用fork()的进程的当前状态完整地复制一遍(除了fork的返回值以外，其余的内存，寄存器等值完全相同)
调用者返回子进程的进程号
新产生的进程返回0

**两个fork的小习题需要掌握一下**


fork-printf.c会打印八个hello
但是把该程序的输出通过管道传给wc -l,会**输出8行**
输出传递给cat，又会输出8个hello

下面这个程序
```
int main(){
    //setbuf(stdout,NULL);
    // 强制stdout的buffer不缓冲
    printf("Hello");
    int *p;
    p=NULL;
    *p=1;
}
```
p没有申请空间，程序会报segfalut错误
但会发现printf的输出也没有显示
如果在hello后面加上\n或者在printf后面加上fflush(stdout)
hello又输出出来了

原因在于printf会把字符先暂存在缓冲区
真正的输出是fflush(stdout)语句完成的
如果stdout指向的是bash，那它的buffer是一个line buffer(碰到换行符输出？main退出还会输出一次)
如果指向pipe，那就是full buffer(写满一定量之后输出一次)

于是如果把fork-printf的输出与管道连接的话
printf执行完毕后不会立即输出，而是依旧保存在缓冲区里面

第一次fork之后，会有两个进程
然后分别执行printf，于是两个进程的缓冲区都有一个hello\n，但不会输出

而fork会把执行的其他库函数状态也给复制出来

第二次fork之后，新生成的两个进程会把父进程的缓冲区也复制
然后再执行printf，于是所有进程缓冲区都会有两个hello\n

main函数返回之后，缓冲区才会输出。
最终有8个hello


#### 如果多线程程序的某个线程调用fork会发生什么

### 通过execve()执行别的进程，不仅仅是复制
这个调用的本质是重置当前进程的状态机
execve(file name,char* argv[],char* envp[])
第二个参数是新进程的参数，其中argv[0]就是file name
第三个参数是指定新进程的环境变量
环境变量不止有PATH

execve会按照path顺序一个个找可以运行的文件

### exit
销毁当前进程
三种用法，其中exit(0)属于libc的库函数
它可以调用atexit

直接用exit的系统调用就只会销毁进程


