# 实现不带 `.b` 后缀指令
---
我们只需要在命令解析中把所有命令全部归一化，让其带上 `.b` 后缀即可：
![[Pasted image 20240621222745.png]]

# 实现指令条件执行
---
- 首先我们需要增加gettoken的功能，使得能够一次性读入两个字符，我们只需要判断当前字符是否为可重复字符，并且判断下个字符是否重复，接着加入一个变量flag记录字符是否重复即可；![[Pasted image 20240622092650.png]]

- 接着我们需要知道函数的返回值，提示是修改exit，但是我们可以发现如果修改exit返回值的话修改的地方可能较多，我初次也进行了一些尝试，但是失败了，所以我选择的是系统调用的方法；
``` cpp
    int result = main(argc, argv);//调用用户的main函数
    syscall_send (result);
    exit();
```
- 然后就是系统调用的实现：
```cpp
void sys_send(int m)
{
    struct Env *e,*e1;
    envid2env(curenv->env_parent_id, &e, 0);
    envid2env(e->env_parent_id, &e1, 0);
    e1->env_flag = m;
}
```
在shell中，我们运行的函数假设为主进程，那么我们是fork了一个子进程去真正执行shell这一行指令，而对于这一行指令的每一小部分，我们再次fork了一个子进程，即==子子进程==来执行；所以，我们传输数据需要传到==当前产生运行进程结果的进程（子子进程）==的父进程的父进程；

下面是我们的具体实现：
![[Pasted image 20240622093941.png]]
对于 `command1 && command2`，如果cmd2不需要执行，那么我们就把他读进去过滤掉就可以了；
# 实现更多指令
---
我们分析过程，也就是需要我们在用户态下实现目录和文件的创建和删除，因为在lab5中已经实现了文件删除的功能，我们只需要在用户态下实现创建目录和文件的功能，也就是通过文件系统的接口来实现文件的创建。

`touch` 和 `mkdir` 创建过程中需要考虑的问题如下：
1. 创建的是目录还是文件；
2. 创建的文件或目录父路径是否存在；
3. 创建的文件或目录是否已经存在；

具体步骤如下：
1. 首先在 `user/lib/file.c` 中实现 `int user_create(char* path, int isdir)` ：这个函数是用户直接调用即可创建文件或目录的函数；
``` cpp
int user_create(char* path, int isdir){
    int r = fsipc_create(path, isdir);
    return r;
}
```
2. 这个函数调用了fsipc也就是文件系统的通信，那么我们就按照步骤实现 `user/lib/fsipc.c` 中的 `int fsipc_create(char* path, int isdir)` ：这个函数就是给`struct Fsreq_create` 变量赋值，并且调用 `fsipc(FSREQ_CREATE, req, 0, &perm)`
``` cpp
int fsipc_create(char* path, int isdir) {
    u_int perm;
    struct Fsreq_create *req;
    req = (struct Fsreq_create *)fsipcbuf;
    if (strlen(path) >= MAXPATHLEN ){
        return -E_BAD_PATH;
    }
    strcpy((char *)req->req_path, path);
    req->req_isdir = isdir;
    return fsipc(FSREQ_CREATE, req, 0, &perm);
}
```
3. 接着我们进入文件系统内核态服务函数 `fs/serv.c` 中，实现 `void serve_create(u_int envid, struct Fsreq_create *rq)` 函数：这个函数就是创建文件函数的一些实现了，主要是调用了 `file_create` ，通过这个函数来实现文件的创建，并通过它的返回值来判断当前路径的一些问题；
``` cpp
void serve_create(u_int envid, struct Fsreq_create *rq) {
    int isdir = rq -> req_isdir;
    struct File *f;
    int r;
    if((r = file_create((char *)rq->req_path, &f)) < 0 ){
        ipc_send(envid, r, 0, 0);
        return;
    }
    f->f_type = isdir;
    ipc_send(envid, 0, 0, 0);
}
```
4. 最后我们进入 `fs/fs.c` 中，实现 `int file_create(char *path, struct File **file)` 函数：这个函数就是创建文件函数的实现了；
	- 如果能够找到文件或者目录路径，就返回 `-E_FILE_EXISTS`；
	- 如果找不到，但是其父目录为空，则返回 `-E_NOT_DIR`；
	- 接着就是进行创建了；
``` cpp
int file_create(char *path, struct File **file) {
    char name[MAXNAMELEN];
    int r;
    struct File *dir, *f;
    if ((r = walk_path(path, &dir, &f, name)) == 0) {
        return -E_FILE_EXISTS;
    }
    if (dir == 0) {
        return -E_NOT_DIR;
    }
    if (r != -E_NOT_FOUND) {
        return r;
    }
    if (dir_alloc_file(dir, &f) < 0) {
        return r;
    }
    strcpy(f->f_name, name);
    *file = f;
    return 0;
}
```
（还需要在头文件中定义相关的函数和结构体）
至此，我们创建文件的功能实现了；

- 于是，我们就能够实现 `touch` 和 `mkdir` 了。
- 对于 `touch.c` ：只需要在找不到目录的时候报错就行了；
``` cpp
int main(int argc, char **argv) {
    if (argc != 2) {
        usage();
    }
    int r;
    if ((r = user_create(argv[1], 0)) == -E_NOT_DIR) {
        printf("touch: cannot touch '%s': No such file or directory\n", argv[1]);
    }
    return 0;
}
```
- 对于 `mkdir.c` ：需要一些判断，如果是 `-p` ，那么我们直接递归创建；如果没有参数，那么我们就判断父目录是否存在与目录是否存在即可；
``` cpp
int main(int argc, char **argv) {
    int r;
    if (argc == 3 && (strcmp(argv[1], "-p") == 0)) {
        r = user_create(argv[2], 1);
    }
    else if (argc == 2)
    {
        r = user_create(argv[1], 1);
        if (r == -E_NOT_DIR) {
            printf("mkdir: cannot create directory '%s': No such file or directory\n", argv[1]);
        } else if (r == -E_FILE_EXISTS) {
            printf("mkdir: cannot create directory '%s': File exists\n", argv[1]);
        }
    } else {
        usage();
    }
    return 0;
}
```
- 最后是 `rm.c` ：因为 `remove` 函数我们在lab5已经实现，只需要实现文件检测即可；其实操作系统已经帮助我们实现了 `int stat(const char *path, struct Stat *stat)` 这个函数，这个函数是判断文件状态函数，也就是判断文件是否存在并且返回文件信息（是否为目录）；
所以我们的实现就比较轻松了：
``` cpp
int main(int argc, char **argv) {
    int r;
    struct Stat st;
    char path[MAXPATHLEN];
    if (argc == 3 && strcmp(argv[1], "-r") == 0) {
        strcpy(path, argv[2]);
        if (((r = stat(path, &st)) < 0)) {
            printf("rm: cannot remove '%s': No such file or directory\n", argv[2]);
            return 0;
        } else {
            remove(argv[2]);
        }
    }
    else if (argc == 3 && strcmp(argv[1], "-rf") == 0) {
        remove(argv[2]);
    }
    else if (argc == 2)
    {
        strcpy(path, argv[1]);
        r = stat(path, &st);
        if (r < 0) {
            printf("rm: cannot remove '%s': No such file or directory\n", argv[1]);
        } else if (st.st_isdir) {
            printf("rm: cannot remove '%s': Is a directory\n", argv[1]);
        } else {
            remove(argv[1]);
        }
    } else {
        usage();
    }
    return 0;
}
```

# 实现反引号 & 注释 & 引号
---
如果我们判断出反引号，那么我们就进行一个新的字符处理，而这个字符处理的方式就是不处理；
检测反引号的方式：
![[Pasted image 20240622094232.png]]
处理方式：
![[Pasted image 20240622094324.png]]

注释、引号和上述方法类似，也是检测到相关字符后进行处理，只不过一个是不处理，一个是当成w类型直接输出；


	这里用了投机取巧的方法，可能不是正规的；
# 实现历史指令
---
历史命令记录于 `/.mosh_history` 文件中。
- 首先在shell启动并执行第一行指令后，会在根目录下创建该文件，并在每一次读取完指令后向其中写入历史命令。当然实现在文件末尾添加内容需要文件系统对于`O_APPEND`的支持。需要对文件系统进行修改。
```cpp
void record_history(char* buf, int n) { 
	if (first_run == 0) { 
		user_create("/.mosh_history", 0); 
		first_run = 1; 
	} // record current cmd into /.mosh_history
	int fd = open("/.mosh_history", O_WRONLY | O_APPEND); 
	int k = write(fd, buf, n); 
	close(fd); 
	fd= open("/.mosh_history", O_WRONLY | O_APPEND); 
	k = write(fd, "\n", 1); 
	close(fd); 
	history_size += 1; 
}
```
- 接着，实现通过上下键切换历史指令需要改写 `parsecmd` 函数，使其在接受到上下键时，清空当前记录的命令缓冲(buffer)，导入特定的历史命令，并在console上回显。

``` cpp
if (buf[i] == 0x41) { 
	flush(buf); 
	historyget(index, history_cmd); 
	strcpy(buf, history_cmd); 
	fwritef(1, " %s", buf); 
	i = strlen(buf)-1; 
	index = ((index-1)<0? (index-1+history_size) : (index-1))%history_size; 
} else if (buf[i] == 0x42) { 
	flush(buf); 
	historyget(index-1, history_cmd); 
	strcpy(buf, history_cmd); 
	fwritef(1, " %s", buf); 
	index = (index+1)%history_size; 
	i = strlen(buf)-1; 
}
```
这里`index`的计算通过mod实现了一个循环，会在`up`和`down`键入后增减，在其他键键入后恢复初值`history_size - 1`，即最后一条历史命令。
# 实现一行多指令
---
实现一行多指令需要前面的是子进程实现前语句的完成，后面的是父进程，同时父进程等待子进程即可。
![[Pasted image 20240622094801.png]]
# 实现追加重定向
---
和重定向的过程类似，需要改变的如下：
1. 打开文件方式为如果没有则创建；
2. 如果存在，那就找到指向文件最后的标识符；
![[Pasted image 20240622095020.png]]
# 实现前后台任务管理
---
实现前后台任务管理，我们进行分析，我们需要解决的问题如下：
1. 后台处理进程：可以设置一个全局标志位记录是否要将当前进程挂起。如果挂起，可以通过修改调用逻辑，让当前shell不用等待该进程；
2. 记录所有的进程：通过一个结构体，记录输入的指令与进程i控制块id，并用一个全局数组存储；
3. 不同进程间数据共享：将这个记录数组放到内核态下，对外提供增删打印等接口，以实现所有的进程公用一张表的目的。

下面是具体实现过程：
1. 首先，在 `user/sh.c` 中修改：定义全局变量 `hangup` 和当前的输入指令 `cur_cmd`，在 `parsecmd` 中，判断当前的 `&` 是否处于末尾，如果处于末尾，就将当前进程挂起到后台，进行系统调用`do_job`，继续执行下边的命令即可；
~~~ cpp
int child = fork();
if (child) {
    do_job(1, 0, child, cur_cmd);
    hangup = 1;
    return parsecmd(argv, rightpipe);
} else {
    hangup = 0;
    return argc;
}
break;
~~~
2. 接着，我们需要实现系统调用 `do_job` ，即将这个后台进程的信息储存起来，以便于 `jobs` 指令的运行；在`env.h`中，声明结构体 `job` ， 在 `env.c` 中定义变量：
~~~cpp
//env.h
extern int job_num;
typedef struct {
    int env_id;
    char cmd[100];
    int is_delete;
} job;
extern job all_job[100];
~~~

~~~cpp
//env.c
int job_num = 1;
job all_job[100];
~~~
3. 然后就是在 `user/lib/syscall_lib.c` 中实现`void syscall_do_job(int type, int job_id, int env_id, char *cmd)`函数：
~~~cpp
void syscall_do_job(int type, int job_id, int env_id, char *cmd) {
    msyscall(SYS_do_job, type, job_id, env_id, cmd);
}
~~~
4. 接着在 `kern/syscall_all.c` 中实现 `void sys_do_job(int type, int job_id, int env_id, char *cmd)` 函数：
~~~cpp
void sys_do_job(int type, int job_id, int env_id, char *cmd) {
    if (type == 1) {
        // 增加一个后台进程
        job j;
        j.env_id = env_id;
        strcpy(j.cmd, cmd);
        all_job[job_num++] = j;
    } else if (type == 2) {
        // 打印已有进程信息
        for (int i = 1; i < job_num; i++) {
            job cur_job = all_job[i];
            int job_id = i;
            int env_id = cur_job.env_id;
            struct Env *target_env = NULL;
            envid2env(env_id, &target_env, 0);
            char *status = target_env == NULL ? "Done" : "Running";
            char *cmd = cur_job.cmd;
            printk("[%d] %-10s 0x%08x %s\n", job_id, status, env_id, cmd);
        }
    }
}
~~~
5. 最后实现 `jobs` 指令，由于我们的mk文件中没有编译 `jobs.c` ，所以我们应该在 `user/sh.c` 中的 `runcmd` 函数中就直接判断当前指令是否为 `jobs` 指令，并通过系统调用实现 `jobs` 指令的功能；
~~~cpp
if (strcmp(argv[0], "jobs") == 0)
    do_job(2, 0, 0, 0)；
~~~

`kill` 指令和 `fg` 指令类似，也是在这个函数判断，并进行相关的逻辑，比较简单，与 `jobs` 指令类似；

在实现这个任务的时候，`sys_cgetc` 系统调用实现可能会导致死循环，所以要对其进行修改为如下才能够避免死循环；
![[Pasted image 20240622104325.png]]