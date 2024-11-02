# 思考题
---
## Thinking 5.1
---
外部设备不断写回带来的cache内信息与实际信息不一致的问题，这会导致错误的读写。
存在差异，串口设备在工作过程中的读写频率远高于IDE磁盘，其会带来更多的缓存错误。
## Thinking 5.2
---
一个磁盘块大小为4KB。单个文件有10个直接指针，最多共1024个指针（间接指针中前10个无效，那么单个文件最大为1024∗4KB=4096KB
## Thinking 5.3
---
1GB
## Thinking 5.4
---
``` cpp
//serv.h
#define PTE_DIRTY 0x0004 脏位
#define SECT_SIZE 512    扇区大小
#define SECT2BLK (BLOCK_SIZE / SECT_SIZE) 一磁盘块的扇区数量
#define DISKMAP 0x10000000  磁盘起始地址
#define DISKMAX 0x40000000  磁盘最终地址

//fs.h
#define BLOCK_SIZE PAGE_SIZE     磁盘块大小
#define BLOCK_SIZE_BIT (BLOCK_SIZE * 8)    磁盘块位数

#define MAXNAMELEN 128    文件名称的最大长度
#define MAXPATHLEN 1024   路径名称的最大长度

#define NDIRECT 10          文件直接指针数量
#define NINDIRECT (BLOCK_SIZE / 4)       文件间接指针数量
#define MAXFILESIZE (NINDIRECT * BLOCK_SIZE)     最大文件大小
#define FILE_STRUCT_SIZE 256       文件结构体数量
#define FILE2BLK (BLOCK_SIZE / sizeof(struct File))     一磁盘块中的文件数量
```
## Thinking 5.5
---
磁盘下需要有文件newmotd，其中包含长度至少为11的字符串
```cpp
int r, fdnum, n;
    char buf[200];
    fdnum = open("/newmotd", O_RDWR | O_ALONE);
	writef("%d\n",fdnum);
    if ((r = fork()) == 0) {
	    n = read(fdnum, buf, 5);
	    writef("[child] buffer is \'%s\'\n", buf);
    } else {
	    n = read(fdnum, buf, 5);
	    writef("[father] buffer is \'%s\'\n", buf);
    }
    while(1);
```
## Thinking 5.6
---
```cpp
 struct Fd {
     u_int fd_dev_id; //外设的id
     u_int fd_offset; //读写的偏移量
     u_int fd_omode;  //打开方式，包括只读、只写、读写等
 }; //Fd用于记录文件的基本信息

struct Filefd {
     struct Fd f_fd; //file descriptor
     u_int f_fileid; //文件的id
     struct File f_file; //真正的文件本身
 }; //记录文件的详细信息

 struct Open {
     struct File *o_file;  //打开的文件的指针
     u_int o_fileid;       //打开的文件的id
     int o_mode;           //打开方式
     struct Filefd *o_ff;  //指向读写位置的指针
 }; //打开文件行为的抽象
```
## Thinking 5.7
---
黑三角箭头搭配黑实线表示的是同步消息，是消息的发送者把进程控制传递给消息的接收者，然后暂停活动等待回应；
两条小线的开箭头和黑色实线表示的是异步消息，不需要等待；
开三角箭头搭配黑色虚线表示的是返回消息，与同步消息结合使用的；
操作系统利用env_ipc_recving实现进程的同步。
# 实验难点
---
本次lab5虽然内容比较多，但是难度并不是很高，大部分代码根据提示都能写出来，还是比较容易写的；
lab5大致包括3部分：
1. 外设控制；
2. 文件系统；
## 外设控制
---
这部分不是很难，和理论课差不多，大致了解了操作系统是如何管理IO接口的，进行外设控制，并且实现了IDE磁盘驱动；
主要包括以下函数：
1. `sys_write_dev` ：这个函数将起始虚拟地址为 va ，长度为 len 字节的一段数据写到起始物理地址为 pa ，长度为 len 字节的物理空间上。
2. `sys_read_dev` ：这个函数将起始物理地址为 pa ，长度为 len 字节的物理空间中的数据拷贝到起始虚拟地址为 va ，长度为 len 字节的空间中。前两个函数共同实现了对设备的读写操作，只能在内核态中进行；
3. `ide_read` ：对每一次磁盘读操作（写操作调用 syscall_write_dev ，读操作调用 syscall_read_dev ）
	- 使用 `wait_ide_ready` 等待 IDE 设备就绪;
	- 向物理地址 `MALTA_IDE_NSECT` 处写入单字节 1，表示设置操作扇区数目为 1;
	- 分四次设置扇区号，并在最后一次一同设置扇区寻址模式和磁盘编号;
	- 向物理地址 `MALTA_IDE_STATUS` 处写入单字节值 `MALTA_IDE_CMD_PIO_READ` ，设置 IDE;
	- 使用 `wait_ide_ready` 等待 IDE 设备就绪;
	- 从物理地址 `MALTA_IDE_DATA` 处以四字节为单位读取数据;
4. `ide_write` ：与上边类似，只不过是对每一次磁盘写操作；
## 文件系统结构
---
包括三个文件：
- fsipc.c 实现了与文件系统服务进程基于 IPC 的交互；
- file.c 实现了文件系统的用户接口；
- fd.c 实现了文件描述符相关操作

文件系统的数据结构以及相关宏在思考题中已经表述；
我们主要看一些重要的函数；
- `diskaddr` 函数用于将数据块编号转换为缓冲区范围内的对应虚拟地址；
- `map_block` 函数用于分配映射磁盘块需要的物理页面:
	- 先使用 `block_is_mapped` 查看 blockno 对应磁盘块是否已被映射到内存，若已经建立了映射，直接返 回 0 即可;
	- 若没有建立映射，则使用 diskaddr 函数将 blockno 转换为对应虚拟地址，并调用 syscall_mem_alloc 函数为该虚拟地址分配一页物理页面;
- `unmap_block` 用于释放用来映射磁盘块的物理页面:
	- 传入的参数为磁盘块编号，首先使用 `block_is_mapped` 函数获取对应磁盘块的虚拟起始地址;
	- 如果该磁盘块不是空闲的（ `block_is_free` 函数）并且该磁盘块缓存被写过（ `block_is_dirty` 函 数），则调用 `write_block` 函数将磁盘块缓存数据写回磁盘;
	- 最后调用 `syscall_mem_unmap` 函数解除对应磁盘块缓存虚拟页面到物理页面的映射;
- `dir_lookup` 函数用于在 dir 指向的文件控制块所代表的目录下寻找名为 name 的文件
	- 遍历目录文件 dir 中的每一个磁盘数据块。（使用 `file_get_block` 获取第 i 个数据块的数据。）
	- 遍历单个磁盘块中保存的所有文件控制块;
	- 判断要查找的文件名 name 与当前文件控制块的 f_name 字段是否相同，若相同则返回该文件控制块;
	- 若未找到则返回 `-E_NOT_FOUND`;

接下来，我们要进行请求文件系统操作，我们的接口是 `fsipc`。![[Pasted image 20240526153119.png]]我认为和系统调用一下，通过一个接口然后对文件系统内部进行用户需要的操作；
有 `open`、`read`、`remove` 等函数；
- open 用于打开指定文件。函数接受文件路径 path 和模式 mode 作为输入参数，并返回文件描述符；
- read 用于读取文件内容，该函数接收一个文件描述符编号 fdnum ，一个缓冲区指针 buf 和一个最大读取字 节数 n 作为输入参数，返回成功读取的字节数并更新文件描述符的偏移量；
- remove 函数用于删除给定路径 path 所对应的文件。该函数调用了 fsipc_remove 函数；
# 实验体会
---
这次实验总体难度不是很大，虽然代码量不少，但是总体来说跟着提示都很简单。
其次，这次实验的原理也是基于理论课的，还是强调了学好理论课的重要性，并且这次实验的许多思想和前几次差不多。比如这次的文件系统就和系统调用的思想类似，给用户态提供一个接口，避免其对内部文件进行操作等等思想都是类似的；
再者，这次实验也让我了解到了文件系统是如何代码实现的，对操作系统有了更深的理解。
最后，通过这次实验，我对操作系统的功能的理解更加深入，没想到我平时普普通通点开一个文件竟然在操作系统内部有这么多的代码运行来支持我进行这样的操作。


