make之后，会生成很多.d文件，这个是用于告诉makefile有哪些依赖，然后去找这些依赖项有哪些修改过，仅编译修改后的文件即可

# userinit()代码分析
userinit()用于生成第一个用户态进程
我们知道可以fork+execve可以通过拷贝当前进程然后重置的方式创建子进程
但第一个进程(没有父进程通过fork)是如何创建的呢？

答案是，由内核通过userinit()来创建一个属于用户进程的页面，同时准备好一段initcode代码

userinit()会把initcode(已经写好的机器指令,它负责通过exec系统调用执行真正的"/init"程序)搬进内存,initcode是由initcode.S编译汇编生成initcode.o再链接生成initcode.out,最后通过objcopy得到纯粹的二进制代码
```
user/initcode.out:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <start>:
   0:   00000517                auipc   a0,0x0
   4:   02450513                addi    a0,a0,36 # 24 <init>
   8:   00000597                auipc   a1,0x0
   c:   02358593                addi    a1,a1,35 # 2b <argv>
  10:   00700893                li      a7,7
  14:   00000073                ecall

0000000000000018 <exit>:
  18:   00200893                li      a7,2
  1c:   00000073                ecall
  20:   ff9ff0ef                jal     ra,18 <exit>

0000000000000024 <init>:
  24:   696e692f                0x696e692f
  28:   0074                    addi    a3,sp,12
        ...

000000000000002b <argv>:
  2b:   0024                    addi    s1,sp,8
  2d:   0000                    unimp
  2f:   0000                    unimp
  31:   0000                    unimp
```

做好相关准备工作后，内核里面就有了一个可以执行的进程
然后scheduler()会遍历内核的进程，找到可用的哪个，切换过去执行
执行的就是initcode代码
这段代码就是负责执行exec("/init")
系统调用由ecall触发

接下来可以看看ecall之后会发生什么

ecall之前，打印$sstatus，看到spp位为0，说明ecall之前处于用户态
ecall之后，打印$scause，显示为8，查手册为Environment call from U-mode

ecall会把$stvec赋给$pc
而$stvec(0x3ffffff000)指向的是用户地址空间中的trmapoline(位于虚拟地址最高处),这里存放着ecall相关处理代码，详见trampoline.S，这段代码是在特权模式下执行的

trampoline.S中的uservec功能是保存ecall前的寄存器现场，然后会**切换页表**，把用户态页表转换成内核态页表(csrw,sfence)，然后jump至usertrap()

上述切换页表等操作，其实是因为操作系统也有自己的环境，当用户程序被调用后，操作系统会把自己的环境保存在该进程的trapframe中
所以ecall回到内核时，也需要从trapframe恢复环境

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

这段代码执行完之后，该进程会进入执行真正的/init