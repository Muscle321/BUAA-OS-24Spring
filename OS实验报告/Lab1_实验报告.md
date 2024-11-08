# 思考题
---
### Thinking 1.1
---
这是反汇编`Helloworld.o`后的文件：
![[Pasted image 20240320204106.png]]
这是反汇编`Hello`后的文件：
![[Pasted image 20240320204224.png]]
`objdump`中参数：
`-D`：从objfile中反汇编所有的`section`;
`-S`：尽可能反汇编出源代码；
### Thinking 1.2
---
1. 解析`target`目录下的`mos`可执行文件；
![[Pasted image 20240320204701.png]]
2. 我们生成的`readelf`文件本质上不是`EXEC(可执行文件)`，他是一个`DYN`类型，而`hello`是可执行文件，所以可以被解析；
![[Pasted image 20240320204804.png]]
![[Pasted image 20240320204814.png]]
### Thinking 1.3
---

	操作系统启动时，首先启动bootloader程序，在stage1阶段初始化硬件设备。此时对于MIPS处理器来说，MIPS体系结构上电时，启动入口的地址为0xBFC00000（或为某一个确定的地址），为一个虚拟地址，在kseg1中。将虚拟地址的高三位清零之后，对应的物理地址为0x1FC00000，从这里开始MIPS的第一条指令，完成初始化基本的硬件设备的工作和为stage2做准备。之后进入stage2阶段的入口函数，BIOS从MBR中读取开机信息，以Linux中的GRUB开机管理程序为例，BIOS加载MBR中的GRUB代码后把CPU交给GRUB，GRUB一步一步加载自身代码，从而识别文件系统，然后将文件系统中的内核镜像文件加载到内存中，此时会有一个内存移动的操作，最后BIOS跳到指定的地址运行，这样便可以保证内核入口被正确跳转到。
# 实验难点
---
### Exercise 1.1
---

	在这个实验中，我们需要了解到段和节这两个概念，并且在每个elf文件中，我们如何找到这两个概念所在的位置；
	我们需要通过ELF文件头地址binary，加上节入口偏移shoff，得到节头表第一项的地址；
	接着我们需要读取节头的大小，然后以第一个节头的指针为基地址，不断累加每个节头的地址，知道遍历完所有的节地址；
	难度不是很大。
### Exercise 1.2
---

	这个练习只需要看懂mmu.h，并且理解各个节（.test, .data, .bss）对应的地址关系就可以了.
### Exercise 1.3
---

	这个练习只需要看懂mmu.h文件即可，很简单；
### Exercise 1.4
---

	这个练习需要熟悉C语言；
	其中第一步找到%，并且调用一个回调函数，将之前的信息打印出来；这里回调函数用到了函数指针，函数指针使得我们操作更加方便，如果未来out函数实现了优化，我们不需要一个一个去修改，只需要修改函数指针，使他指向这个回调函数即可；
	接下来每一步都是按部就班查找，最后的%d格式输出也是参考下边的%u,o等生成即可，也很简单。
# 实验体会
---

	总的来说，这次实验的难度并不是很高，但是明显感觉到了和C语言知识关系比较密切。尤其是函数指针，结构体指针等指针的妙用，十分巧妙；
	我觉得我还是要再次复习一下C语言的用法及许多基础知识，读懂代码非常容易，但是要我写出这样的代码，对我这种C语言基础都不是很牢固的人来说也是有一点难度的；
	我理解了段和节，以及C语言编译过程的许多知识，我也了解到了OS内核的启动过程；
	我深刻体会到了一个printf函数竟然如此的复杂，也知道了Python教科书中作者说的，如果Hello World！能正确输出在console中，那么证明系统编译等等环节都是正确无误的。