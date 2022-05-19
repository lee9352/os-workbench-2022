先弄了个状态机示例
在vim里面直接编译运行
**:!gcc %**，%表示的是该文件完整路径名
** %! bash command ** 表示把vim的内容作为标准输入，输入给command，并把command的结果覆盖掉当前vim文件
gcc -E可以把宏展开

### 程序就是状态机
C程序状态机模型(源代码视角)
- 状态 = 堆 + 栈(很多个stack frame(每个frame有PC) + 全局变量)
- 初始状态 = main的第一条语句(初始化全局变量)
- 迁移 = 执行一条C语句(执行当前stack frame PC的语句;PC++)
    - 函数调用 = push frame
    - 函数返回 = pop frame

二进制代码视角
- 状态 = 内存M + 寄存器R
- 初始状态 = 
- 迁移 = 执行一条指令(如果只有计算指令，程序甚至没办法退出，会一直执行内存中的指令(假设指令全部合法) )

系统调用指令(syscall)，把状态机交给操作系统，任其修改，或者与系统中的对象交互


### 最小的hello world
直接编译运行？得到的是个动态可执行文件
-static？可执行文件有871Bytes，0.8MB
-c仅编译，可以得到hello.o(目标文件)，看上去很短，但是没法链接成可执行文件
    把main换成_start，又提示找不到printf
把start变成空函数，可以链接成功，运行得到segmentation fault
把start函数改成死循环，可以成功运行
利用gdb调试工具，发现retq执行出错
    retq行为：M[rsp]->rip, rsp+8->rsp
    以栈顶元素为地址，去内存中取出函数退出后要跳转的pc赋值给pc寄存器，栈顶退8字节
    打印rsp，发现rsp为1，执行retq提示：cannot access addr 1
    需要找到syscall，正确退出程序
我猜编译器只对main函数的return使用syscall退出**猜错了**
那就应该是libc啥的设定好了初始状态，以及退出函数，然后再把main函数压栈。main执行完毕出栈后，就进入了退出代码
syscall在libc中实现


### 两种视角转换，就是编译器
编译只确保c语言中不可优化的部分语义完全一致
保证观测一致性的前提下优化改写代码
