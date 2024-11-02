#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()#"

int flag = 0;

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
// / *概述:
// *从s处的字符串中解析下一个token。
// ＊
// 后置条件:
// *将'*p1'设置为令牌的开头，'*p2'设置为令牌的后面。
// *返回:
// * - 0，如果到达字符串的末尾。
// * - '<'表示<(标准输入重定向)。
// * - '>' for >(标准输出重定向)。
// * - '|'表示|(管道)。
// * -“w”表示单词(命令、参数或文件名)。
// ＊
// *缓冲区被修改为将单词后的空格转换为零字节('\0')，以便
// *返回的令牌是一个以空结尾的字符串。
// * /
// 这个函数是为了实现shell的输入解析，将输入的字符串解析成一个个的token，返回token的类型,其中输入的参数为s，p1，p2，s为输入的字符串，p1为token的起始位置，p2为token的结束位置

void runcmd(char *s);

void do_job(int type, int job_id, int env_id, char *cmd) {
	syscall_do_job(type, job_id, env_id, cmd);
}
int hangup = 0;
char cur_cmd[100];

int _gettoken(char *s, char **p1, char **p2)
{

	*p1 = 0;
	*p2 = 0;
	if (s == 0)
	{
		return 0;
	}

	while (strchr(WHITESPACE, *s))
	{
		*s++ = 0;
	}
	if (*s == 0)
	{
		return 0;
	}
	if (*s == '\"')
	{
	 	*s = 0;
	 	*p1 = ++s;
	 	while (*s != 0 && *s != '\"')
	 	{
	 		s++;
	 	}
	 	if (*s == 0)
	 	{
	 		printf("wrong cmd!\n");
	 		return 0;
	 	}
	 	*s = 0;
	 	*p2 = ++s;
	 	return 'w';
	}

	if (*s == '`')
	{
	 	*s = 0;
	 	*p1 = ++s;
	 	while (*s != 0 && *s != '`')
	 	{
	 		s++;
	 	}
	 	if (*s == 0)
	 	{
	 		printf("wrong cmd!\n");
	 		return 0;
	 	}
	 	*s = 0;
	 	*p2 = ++s;
	 	return '`';
	}
	if (strchr(SYMBOLS, *s))
	{
		int t = *s;
		*p1 = s;
		if (*s == *(s + 1))
		{
			flag = 1;
			*s++ = 0;
			*s++ = 0;
		}
		else
		{
			flag = 0;
			*s++ = 0;
		}
		*p2 = s;
		return t;
	}



	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s))
	{
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1)
{
	static int c, nc;
	static char *np1, *np2;

	if (s)
	{
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe)
{
	int argc = 0;
	int fid;
	while (1)
	{
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		switch (c)
		{
		case 0:
			return argc;
		
		case 'w':
			if (argc >= MAXARGS)
			{
				debugf("too many arguments\n");
				exit();
			} // 这种情况是token为一个单词，将这个单词存入argv数组中
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w')
			{
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			// If the 'open' function encounters an error,
			// utilize 'debugf' to print relevant messages,
			// and subsequently terminate the process using 'exit'.
			/* Exercise 6.5: Your code here. (1/3) */
			fd = open(t, O_RDONLY);
			if (fd < 0)
			{
				debugf("failed to open '%s'\n", t);
				exit();
			}
			dup(fd, 0);
			close(fd);
			// user_panic("< redirection not implemented");

			break;
		case '>':
			if (flag ==0)//"<"
			{
				if (gettoken(0, &t) != 'w')
				{
					debugf("syntax error: > not followed by word\n");
					exit();
				}
				// Open 't' for writing, create it if not exist and trunc it if exist, dup
				// it onto fd 1, and then close the original fd.
				// If the 'open' function encounters an error,
				// utilize 'debugf' to print relevant messages,
				// and subsequently terminate the process using 'exit'.
				/* Exercise 6.5: Your code here. (2/3) */
				fd = open(t, O_WRONLY);
				if (fd < 0)
				{
					debugf("failed to open '%s'\n", t);
					exit();
				}
				dup(fd, 1);
				close(fd);
				// user_panic("> redirection not implemented");
				break;
			}
			else//">>"
			{
				if (gettoken(0, &t) != 'w')
				{
					debugf("syntax error: >> not followed by word\n");
					exit();
				}
				fd = open(t, O_WRONLY|O_CREAT);
				if (fd < 0)
				{
					debugf("failed to open '%s' for appending\n", t);
					exit();
				}
				struct Stat mystat;
				stat(t,&mystat);
				seek(fd, mystat.st_size);
				dup(fd, 1);
				close(fd);
				break;
			}
		case '`':
			runcmd(t);
			break;
		case ';':
			fid = fork();
			if (fid == 0)
			{
				return argc;
			}
			else
			{
				wait(fid);
				return parsecmd(argv, rightpipe);
			}
			break;
		case '&':
			if (flag) //"&&"
			{
				fid = fork();
				if (fid == 0)
				{
					return argc;
				}
				else
				{
					wait(fid);
					if (env->env_flag == 0)
					{
						return parsecmd(argv, rightpipe);
					}
					else
					{
						char *tmp;
						c = gettoken(0, &t);
						return parsecmd(argv, rightpipe);
					}
				}
			}
			else //"&"
			{
				int child = fork();
				if (child) {
					do_job(1, 0, child, cur_cmd);
					hangup = 1;
					return parsecmd(argv, rightpipe);
				} else {
					hangup = 0;
					return argc;
				}
			}
			break;
		case '#':
			return argc;


		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			if (flag == 1) // "||"
			{
				// debugf("this is &&");
				fid = fork();
				if (fid == 0)
				{
					return argc;
				}
				else
				{
					wait(fid);
					// printf("flag=%d\n",env->env_flag );
					if (env->env_flag == 1)
					{
						return parsecmd(argv, rightpipe);
					}
					else
					{
						char *tmp;
						c = gettoken(0, &t);
						// printf ("ch = %d\n",c);
						return parsecmd(argv, rightpipe);
					}
				}
			}
			else // "|"
			{
				int p[2];
				/* Exercise 6.5: Your code here. (3/3) */
				pipe(p);
				*rightpipe = fork();
				if (*rightpipe == 0) // 这是子进程
				{
					dup(p[0], 0);
					close(p[0]);
					close(p[1]);
					return parsecmd(argv, rightpipe);
				}
				else // 这是父进程
				{
					dup(p[1], 1);
					close(p[1]);
					close(p[0]);
					return argc;
				}
				// user_panic("| not implemented");
				break;
			}
		}
	}
	return argc;
}

void runcmd(char *s) {
	strcpy(cur_cmd, s);
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;
	if (strcmp(argv[0], "jobs") == 0) {
		do_job(2, 0, 0, 0);	
	} else {
	
		int child = spawn(argv[0], argv);
		close_all();
		if (child >= 0) {
			if (!hangup) wait(child);
		} else {
			debugf("spawn %s: %d\n", argv[0], child);
		}

	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

void readline(char *buf, u_int n)
{
	int r;
	for (int i = 0; i < n; i++)
	{
		if ((r = read(0, buf + i, 1)) != 1)
		{
			if (r < 0)
			{
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if (buf[i] == '\b' || buf[i] == 0x7f)
		{
			if (i > 0)
			{
				i -= 2;
			}
			else
			{
				i = -1;
			}
			if (buf[i] != '\b')
			{
				printf("\b");
			}
		}
		if (buf[i] == '\r' || buf[i] == '\n')
		{
			buf[i] = 0;
			return;
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n')
	{
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void)
{
	printf("usage: sh [-ix] [script-file]\n");
	exit();
}

int main(int argc, char **argv)
{
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	printf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	printf("::                                                         ::\n");
	printf("::                     MOS Shell 2024                      ::\n");
	printf("::                                                         ::\n");
	printf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN
	{
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1)
	{
		usage();
	}
	if (argc == 1)
	{
		close(0);
		if ((r = open(argv[0], O_RDONLY)) < 0)
		{
			user_panic("open %s: %d", argv[0], r);
		}
		user_assert(r == 0);
	}
	for (;;)
	{
		if (interactive)
		{
			printf("\n$ ");
		}
		readline(buf, sizeof buf);

		if (buf[0] == '#')
		{
			continue;
		}
		if (echocmds)
		{
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0)
		{
			user_panic("fork: %d", r);
		}
		if (r == 0)
		{
			runcmd(buf);
			exit();
		}
		else
		{
			wait(r);
		}
	}
	return 0;
}
