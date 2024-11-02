# 思考题
---
### Thinking 0.1
---
Untracked.txt内容：
![[Pasted image 20240308195345.png]]
Stage.txt内容：
![[Pasted image 20240308195439.png]]
Modified.txt内容：
![[Pasted image 20240308195503.png]]
`Untracked.txt`中显示的`README.txt`文件为未追踪状态；
`Stage.txt`中显示的`README.txt`文件为已暂存状态；
`Modified.txt`中显示的`README.txt`文件为已修改状态，但是尚未暂存；
这三种不同的提示分别意味着：在 `README.txt`新建的时候，其处于为未跟踪状态 （untracked）；在 `README.txt` 中任意添加内容，接着用 `add` 命令之后，文件处于暂存状 态（staged）；在修改 README.txt 之后，其处于被修改状态（modified）。
### Thinking 0.2
---
`add the file` 对应 `git add`;
`stage the file` 对应 `git add`;
`commit` 对应 `git commit`;
### Thinking 0.3
---
1. 使用`git checkout print.c`；
2. 使用 `git reset HEAD print.c` 以及 `git checkout print.c`；
3. 使用 `git reset HEAD hello.txt`;
### Thinking 0.4
---
第一次提交日志：
![[Pasted image 20240308204545.png]]
第二次提交日志：观察到第三次提交消失
![[Pasted image 20240308204951.png]]
第三次提交日志：观察到仅剩第一次提交的
![[Pasted image 20240308205320.png]]
第四次提交日志：三次全部回来（其中的哈希值为提交说明为3的哈希值）
![[Pasted image 20240308205529.png]]
### Thinking 0.5
---
![[Pasted image 20240308211615.png]]
### Thinking 0.6
---
command内容：
![[Pasted image 20240308213922.png]]
result文件内容：
![[Pasted image 20240308214022.png]]
解释说明：

	定义了三个变量a、b、c，a赋值为1，b赋值为2，c赋值为a+b即为3。将a、b、c分别存入file1、file2、file3中，再依此定向写入file4中，最后将file4中结果存入result
过程代码：
![[Pasted image 20240308214148.png]]
1. `echo echo Shell Start` 与 `echo echo Shell Start` 效果没有区别
2. `echo echo $c>file1` 与 `echo echo $c>file1` 效果有区别; result中没有3，只有2和1；其次`./test`后的`save c to ./file1`后多空了一行；
![[Pasted image 20240308214745.png]]
### Exercise 0.1
---
1. 第一题没什么难度，C语言知识；
2. 第二题中的Makefile也是最基本的知识；
3. 第三题使用了`sed`命令以及输出重定向，也是非常简单基本的用法；
4. 第四题就是最简单的`cp`指令了；
总的来说第一题没什么难度。
### Exercise 0.2
---
非常简单的`shell`脚本的编写，很基本；
注意a的添加方式，即：`let a=a+1` 或 `a=$((a+1))`;
### Exercise 0.3
---
这个题目难在我们不知道`grep`指令打出来的结果是什么；所以我先用`grep -n`指令查找了一下，发现分隔符为`:`；于是我们查找`awk`指令的用法，利用`awk -F ':'`将其分隔开，并且输出重定向到输出文件；
### Exercise 0.4
---
1. 这个题很简单，但是有个坑，就是我们在`sed`指令中如果使用字符串需要用 `'""'` (sed命令使用单引号)或者 `直接使用变量$var`（sed命令使用双引号） ，即： `sed -i 's/'"$2"'/'"$3"'/g' $1` ；
2. 第二题有点恶心，这里在 `csc/code` 目录下的 `Makefile` 中，需要引用的头文件不在该目录下，所以就需要使用 `gcc` 引用头文件的编译方法，即需要 `gcc -i ../include -c %.c`来进行链接头文件并且编译 `.c` 文件，注意是此时仅仅编译但不链接；并且在该 `Makefile` 中直接向父目录生成可执行文件 `fibo` ；在`csc/`下的 `Makefile` 中，我们需要使用 `make -C <subdir>` 来执行子目录下的 `Makefile`文件。
# 难点分析
---

	1. 熟练掌握了基本的Linux的基础操作，如ls，touch，mkdir，cd, rmdir, rm, cp, mv, man;
	2. 熟练运用vim的相关操作；
	3. 熟练运用GCC编译器；
		1. 可以使用-I来指定头文件目录；
		2. 可以使用-o指定生成的输出文件；
		3. -c是只编译不链接；
	4. 熟练应用了Makefile文件；
	5. 学习了Git的相关操作和知识
		1. Git的四种文件状态；
		2. Git的操作指令，git add，git commit， git pull， git push等等操作；
	6. 学习了find，grep，tree，chmod，diff，sed，awk等进阶Linux操作；
	7. 学会了tmux窗格操作；
	8. 熟练应用了shell脚本操作；
		1. 需要注意的是shell中空格的使用；
		2. 需要注意循环和选择语句的条件；
	9. 掌握了重定向和管道的知识；
# 实验体会
---

	1. 通过此次实验，我了解到了许多linux操作系统的知识，我也掌握了许多实用工具的使用，了解了实验环境，了解控制终端，熟悉了许多操作。同时，我也自行上网搜索，了解到了许多操作系统的知识，为以后的实验打好基础！
	2. 对于awk， grep， sed这三个扩展指令，我在网上搜了许多相关知识，学习了他们的一些操作用法（教程里没有的），这次实验也锻炼了自己检索的能力。