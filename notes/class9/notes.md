# 开机第一条指令
cpu上电会有reset信号设置rip寄存器(就是pc寄存器)
主板上会把内存某一块固定的地址指向主板的一个固件firmware
于是cpu一上电就会先去执行固件代码
这个代码会扫描所有的磁盘分区

## Legacy BIOS
判断哪些磁盘分区有主引导区(MBR，位于启动盘最开始处，仅512B)
判断方式就是读取每个磁盘的前512字节，看最后两个字节是否为55aa
接着就会把MBR搬到内存0x7c00处，再跳转执行MBR的代码(本质是一个加载器(boot loader)，用于后续真正加载内核)

### firmware病毒
奔腾cpu允许修改fireware，这本来是用于更新固件的
于是就可以篡改fireware
导致无法开机

## UEFI
磁盘必须预留FAT32分区
支持更多设备驱动(比如在bios里面可以用鼠标)


# Lab0跑通
把os-workbench-2022切换到L0分支(在~/os/os-workbench-2022里面)
进入amgame文件夹运行make，make全过程记录在amgame/build/make.log里面
然后在docker里面执行命令qemu-system-x86_64 -drive format=raw,file=amgame-x86_64-qemu即可

## thread-os.c
其实在L0分支下新建一个文件夹， 把thread-os.c放进去，然后修改一下amgame底下的makefile，就能生成thread-os了

但是由于我没有实现klib底下的printf，所以暂时不能正确的执行