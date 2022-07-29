驱动程序就是一段内核代码

linux中所有设备都是文件
文件就有与文件相关的系统调用(read,write...)
驱动就是把这些系统调用**翻译成与设备的寄存器交互**

关于head -c 512 /dev/zero没有输出
原因是/dev/zero里面的0是\0，在字符串当中属于终止符，所以不显示输出
必须head -c 512 /dev/zero | xxd
让xxd进行16进制显示
\0对应的asic码为0

io设备的控制由系统调用ioctl负责完成
int ioctl(int fd, unsigned long request, ...);
可以直接给设备发送各种命令