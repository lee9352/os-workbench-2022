make之后，会生成很多.d文件，这个是用于告诉makefile有哪些依赖，然后去找这些依赖项有哪些修改过，仅编译修改后的文件即可

# userinit()代码分析
userinit()用于生成第一个用户态进程
我们知道可以fork+execve可以通过拷贝当前进程然后重置的方式创建子进程
但第一个进程(没有父进程通过fork)是如何创建的呢？

答案是，由内核通过userinit()来创建一个属于用户进程的页面，同时准备好一段initcode代码

userinit()会把initcode(已经写好的机器指令,它负责通过exec系统调用执行真正的"/init"程序)搬进内存,initcode是由initcode.S编译汇编生成initcode.o再链接生成initcode.out,最后通过objcopy得到纯粹的二进制代码


做好相关准备工作后，内核里面就有了一个可以执行的进程
然后scheduler()会遍历内核的进程，找到可用的哪个，切换过去执行
执行的就是initcode代码
这段代码就是负责执行exec("/init")
系统调用由ecall触发

# ecall
接下来可以看看ecall之后会发生什么

ecall之前，打印$sstatus，看到spp位为0，说明ecall之前处于用户态
ecall之后，打印$scause，显示为8，查手册为Environment call from U-mode

ecall会把$stvec赋给$pc
而$stvec(0x3ffffff000)指向的是用户地址空间中的trmapoline(位于虚拟地址最高处),这里存放着ecall相关处理代码，详见trampoline.S，这段代码是在特权模式下执行的

trampoline.S中的uservec功能是保存ecall前的寄存器现场，然后会**切换页表**，把用户态页表转换成内核态页表(csrw,sfence)，然后jump至usertrap()

上述切换页表等操作，其实是因为操作系统也有自己的环境。执行用户程序前，操作系统会把自己的环境(内核栈位置，$satp寄存器)保存在该进程的trapframe中
所以ecall回到内核时，也会从trapframe恢复环境

切换至内核页表后，在qemu中info mem，看到所有页表的虚拟地址就是物理地址(操作系统能够看到所有的真实页表)

riscv有一个寄存器$satp用于管理各个进程的地址空间
这个寄存器指向了一个内存地址
也就是说页表信息都是存放在内存中的（debug的时候无法访问）
每次切换环境时，都需要csrw $satp,$reg


usertrap()会检查sstatus的spp位为0
```
  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");
```

usertrap判断这是一条用户态的trap指令后，会执行syscall()
syscall根据a7寄存器的系统调用编号，判断出这是一个exec系统调用
然后跳转去执行sys_exec()
调试时通过(char*)path打印出要执行的程序为/init

然后执行exec()**这里才真正地开始读取/init文件，妈了个巴子**

执行完毕后，通过usertrapret()，然后调用trampoline.S的userret返回至用户态

这段代码执行完之后，该进程会执行真正的/init

# 关于sheduler和swtch
uerinit()创建进程时，会调用一个allocproc()
这一步会把proc的context的ra置为forkret,forkret又会执行trapret(第一个进程被调度后，会从内核forkret处开始返回)

scheduler()本身是一个调度函数，它负责找到一个可以执行的进程
然后调用swtch()函数，（调用swtch前，编译器会负责保存一部分寄存器至栈中）
于是这个函数只需要把当前ra和sp，及部分寄存器存入cpu.context。**这部分上下文+栈中的寄存器，构成了scheduler从swtch返回时应该具备的环境**
然后把p.context放入寄存器。**这一步修改了ra和sp寄存器**
于是函数return的时候，就并没有跳转到scheduler处，而是跳转到了forkret去啦！
接着便开始执行trapret(这里才是真正的恢复进程现场，同时保存kernel寄存器至trapframe)。trapret执行后，才会转为用户态执行用户程序



# 进程切换
照着上面的流程接着走
当进程执行到一半，来了个中断需要切换进程
也会先陷入内核执行trampoline：保存所有的寄存器至trapframe，并从trapframe中取出kernel环境
然后执行usertrap()
它会调用yield()让出cpu
yield最终也会调用swtch(),保存yield当前环境，然后恢复cpu.context(恢复成了从scheduler进入swtch，准备返回前的状态)
swtch返回时就变成了**好像刚从scheduler调用swtch函数然后返回时的状态**，然后接着执行scheduler调度程序

scheduler会修改current process的值然后再次执行swtch()，于是又返回到了yield()，然后执行usertrap()
usertrap又跳转到usertrapret，这里会根据current process来恢复新的进程环境

**process.context保存的是该进程进入内核后，一直运行到进入swtch函数时的上下文**
总结一下，就是sheduler第一次执行swtch时，会保存自身，swtch返回时会跳转到forkret去执行
之后的所有调度，进程都是会陷入内核usertrap，一直执行到yield，然后执行swtch函数，但是函数返回时又跳转到sheduler，简单的修改了一下current preocess，然后又通过swtch返回来执回usertrapret（恢复成另一个进程的状态）

整个过程如下：一次进程切换需要两次swtch
trap(执行swtch)->scheduler(执行swtch)->trap

**可以把usertrap函数和scheduler函数想象成两个协程**，他们通过swtch来回切换
scheduler负责修改current process，
返回至trap后，它负责从内核返回用户进程(把trapframe中的epc放入sepc寄存器，调用sret时会把sepc赋给pc)
但这个process已经不是刚刚陷入内核的那个进程了，而是scheduler从进程池中选择的一个新进程

