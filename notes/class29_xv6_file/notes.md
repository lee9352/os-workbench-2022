mkfs.c的作用是创建一个文件系统
直接读有点痛苦
jyy写了个脚本，可以查看运行时的backtrace
gdb -ex 'source mkfs/trace.py' mkfs/mkfs
注:**为了正确调试，需要重新编译mkfs,并添加-Og选项，否则进入函数调用后，参数不会马上更新，必须再step一次才会更新函数的参数**
让我们了解运行过程中函数调用信息

所有的函数调用最后都指向两个函数
rsect和wsect，即读磁盘和写磁盘


这里面有一点低效，例如刚开始往根目录写第一个目录项时，调用winode(inum=1)
    它会执行rsect(sec=32),然后wsect(sec=32) //这里是修改inode一些信息

接着执行append(inum=1, n=16),就是往inode1写16个字节
    他又去执行rsect(sec=32)，又先把inode1读出来，目的是找到对应数据再磁盘中的位置

然后执行rsect(sec=46),再wsect(sec=46)，把真正的数据部分修改完毕

然后还需要修改inode，又有一次读写磁盘的操作

仅仅添加一个目录项，就有这么多次读写磁盘操作，性能会很拉跨

所以当系统跑起来之后，需要给文件系统实现一个buffer(kernel/bio.c)，每次的读写不用真的访问磁盘,而是先在内存里面查看是否有缓存

上述buffer只有内核跑起来了才能用，mkfs的性能就是很低

fs使用bread,调用bget访问buffer cache
在buffer cache内部遍历整个cache，找是否有对应块
如果没有，bget返回的结构体valid为false
于是再调用virtio_disk_rw访问文件系统

写文件时，先执行writei写日志
这里会先bread读出日志内容，然后log_write修改
**以上操作都是在内存中的操作，日志并没有真正写入**

上述操作完成后，在end_op里面有一个commit操作
这里才会执行write_log写日志

确保日志写完后，执行write_head，把日志头补上，表示日志写入完毕
这样子如果在写日志过程中崩溃，head没有写入，这次操作就直接丢掉。只有head写入了，才表示这个日志生效了

然后调用install_trans把日志写入文件系统