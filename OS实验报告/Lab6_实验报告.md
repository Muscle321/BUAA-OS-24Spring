# 思考题
---
## Thinking 6.1
---
```cpp
修改后代码如下：
test_pipe.c
#include <stdlib.h>
#include <unistd.h>

int fildes[2];
char buf[100];
int status;

int main(){
	status = pipe(fildes)；
	if (status == -1 ) {
		printf("error\n");
	}
	switch (fork()) {
	case -1:
		break;
	
	case 0: /* 子进程 - 作为管道的写者 */
		close(fildes[0]); /* 关闭不用的读端 */
		write(fildes[1], "Hello world\n", 12); /* 向管道中写数据 */
		close(fildes[1]); /* 写入结束，关闭写端 */
		exit(EXIT_SUCCESS);
	
	default: /* 父进程 - 作为管道的读者 */
	close(fildes[1]); /* 关闭不用的写端 */
	read(fildes[0], buf, 100); /* 从管道中读数据 */
	printf("child-process read:%s",buf); /* 打印读到的数据 */
	close(fildes[0]); /* 读取结束，关闭读端 */
	exit(EXIT_SUCCESS);
	}
}
```
## Thinking 6.2
---
假如在执行完 `syscall_mem_map(0, oldfd, 0, newfd, vpt[VPN(oldfd)] & (PTE_D | PTE_LIBRARY))` 语句之后立刻切换到该管道的另一个同为读或写的进程执行，此时该进程通过 `pageref(pipe)` 得到pipe页面的引用次数为2，又因为 `pageref(oldfd)` 也变成了2，两者相等，该进程调用 `_pipe_is_closed` 后得出管道另一侧关闭的结论，从而造成程序运行错误。
## Thinking 6.3
---
因为进入系统调用时系统会陷入内核态，此时系统会关闭所有中断来防止其他进程的干扰，具体来说就是设置SR寄存器的中断屏蔽位。因此所有的系统调用一定是原子操作。
## Thinking 6.4
---
1、可以解决。因为在正常情况下pageref(fd) < pageref(pipe)，只要我们在解除映射时先解除fd的映射，就能保证pageref(fd)更小，不会存在两者相等的中间情况，因此可以解决上述场景竞争问题。

2、会出现类似问题。
原因：假如在执行完 `syscall_mem_map(0, oldfd, 0, newfd, vpt[VPN(oldfd)] & (PTE_D | PTE_LIBRARY))` 语句之后立刻切换到该管道的另一个同为读或写的进程执行，此时该进程通过 `pageref(pipe)` 得到pipe页面的引用次数为2，又因为pageref(oldfd)也变成了2，两者相等，该进程调用 `_pipe_is_closed` 后得出管道另一侧关闭的结论，从而造成程序运行错误。
解决方案：与close中的解决方案类似，我们只要保证不会出现pageref(fd) == pageref(pipe)的中间情况即可，因此只需要让引用次数更多的pipe首先映射，实现“大者更大”，这样就不可能出现两者相等的中间情况。
## Thinking 6.5
---
对于bss在ELF中不占空间，但是在内存中占据空间的问题，是在elf_load_seg函数中实现的，在该函数中，对于bin_size < sgsize的情况，直接用0填充使得程序占据空间达到sgsize，这就使得bss中所有的值被赋为0；
```cpp
while(i < sgsize) {
	if ((r = map_page(data, va + i, 0, perm, NULL, MIN(bin_size - i, BY2PG))) != 0) {
	return r;
	}
	i += PAGE_SIZE;
}
```
## Thinking 6.6
---
是在user/init.c中实现的，具体代码如下：
```cpp
if ((r = opencons()) != 0) {
	user_panic("opencons: %d", r);
}
if ((r = dup(0, 1)) < 0) {
	user_panic("dup: %d", r);
}
```
## Thinking 6.7
---
在MOS中我们用到的shell命令是外部命令，因为我们在解析和执行命令时会fork一个子shell。
linux的cd命令之所以为内部命令，是因为cd命令不是一个独立的可执行文件，而是由shell的一个内置函数实现的，在解析到cd命令时，只需要调用该函数即可，这样可以提高系统的运行效率。
## Thinking 6.8
---
输出如下：![[Pasted image 20240527195123.png]]
1. 可以观察到2次spawn，分别对应3805和4006进程，也就是ls.b命令和cat.b命令的两个进程；
2. 可以观察到4次进程销毁，分别对应3805、4006、3004、2803四个进程，即ls.b命令进程、cat.b命令进程、管道fork出的子shell进程以及开始main函数中fork出的子shell进程；
# 难点分析
---
本次实验主要包括两部分，一部分是实现管道，一部分是实现shell；
其中前者是后者的基础，因为shell大部分操作是靠管道实现的；
总体来说，挺好理解，难度也不大，大部分代码已经给出；并且这次实验也考察了对lab3进程调度相关的知识；
## 管道
---
管道，抽象的来看就是一块内存，封装出两个端口，每一端口都可以读或者写，这些由初始化的时候进行定义；![[Pasted image 20240527200750.png]]
管道主要包括6个函数：
- 创建管道函数 `int pipe(int pfd[2])`：这个函数用于创建管道，两个端口，其中fd0只读，fd1只写，通过函数的传入参数将这两个文件描述符返回；
- 查询关闭函数 `static int _pipe_is_closed(struct Fd *fd, struct Pipe *p)` 和 `int pipe_is_closed(int fdnum)` ：这两个函数用于判断管道是否已经被关闭；
- 读管道函数 `static int pipe_read(struct Fd *fd, void *vbuf, u_int n, u_int offset)` ：这个函数是从fd对应的管道数据缓冲区中，读取至多 n 字节到 vbuf 对应的虚拟地址中，并返回本次读到的字节数；
- 写管道函数 `static int pipe_write(struct Fd *fd, const void *vbuf, u_int n, u_int offset)`：这个函数的作用是从 vbuf 对应的虚拟地址，向 fd 对应的管道数据缓冲区中写入 n 字节，并返回本次写入的字节数；
- 关闭管道函数 `static int pipe_close(struct Fd *fd)` ：这个函数的作用是关闭管道的一端；
## shell部分
---
shell就是操作系统给用户者提供的一个”外壳”，用户可以通过shell进行操作。在我们的实验中，我们只需要实现命令行式的shell。下面是shell的启动执行过程![[Pasted image 20240527201511.png]]
这里边包含许多重要的函数：
- 初始化栈空间函数`int init_stack(u_int child, char **argv, u_int *init_esp)`：这个函数主要是初始化子进程的栈空间，达到向子进程的主函数传递参数的目的。
- `int spawn(char *prog, char **argv)`函数：这个和函数和fork类似，都是产生一个子进程，但是这个函数产生的子进程不再执行与父进程相同的程序，而是装载新的ELF文件，执行新的程序；
- sh.c中的主函数`int main(int argc, char **argv)`：这个函数的主体就是一个循环，不断调用 `readline` 读取用户输入的命令，然后 `fork` 出一个子进程，子进程执行用户的命令，执行结束后子进程结束。父进程在此等待子进程结束后，返回循环开始，读入用户的下一个命令；
- 命令读入函数 `void readline(char *buf, u_int n)`：从标准输入（控制台）中读取一行用户读入的命令，保存在 `char *buf` 中；
- 命令解析函数 `int _gettoken(char *s, char **p1, char **p2)` 和 `int gettoken(char *s, char **p1)` ：这两个函数用于将命令字符串分割，解析命令中的特殊符号；
- cmd命令解析函数 `void parsecmd(char **argv, int *rightpipe)` 和 `void runcmd(char *s)`：前者是解析用户输入的命令，后者则是进行命令的执行；
# 实验体会
---
本次实验作为OS课设的收官之作，比前面任何一次实验都更加关注操作系统的全局性，同时抽象的程度相比之下也最高。无论是管道的实现还是shell的实现，这些都是为了直接面向用户的，只有经过了这次实验，我才真正知道了操作系统给用户提供的接口是如何实现的，还有我们在进行用户端操作的时候，对操作系统在背后做的事情有了更多的认识；

这次实验难度也不是很大，但是十分有意思，也许是真的把一个操作系统做完了，所以很有成就感，在用自己写的操作系统执行一系列命令的时候，很有趣也很爽。
当然，对于操作系统的具体实现，我也是有了更多的认识和思考，真的挺有意思的，平时在使用电脑的时候，也会去思考，当我进行这个命令的时候，操作系统会在底层进行什么操作呢？

再者，在这次实验也是和理论知识结合了，当然结合的理论知识也是刚开学学习的shell和管道机制，不得不说这些机制抽象出来真的太有用了，把他封装成两个接口也是十分好用。完成这次实验，我对面向对象的思想也有了更新一步的认识；

最后，感谢操作系统课程让我动手实现了一个自己做的小型操作系统，虽然很难但是很有成就感！谢谢课程组和助教老师们！