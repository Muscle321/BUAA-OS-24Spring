#include <env.h>
#include <lib.h>
#include <mmu.h>

void exit(void)
{
	// After fs is ready (lab5), all our open files should be closed before dying.
#if !defined(LAB) || LAB >= 5
	close_all();
#endif

	syscall_env_destroy(0);
	user_panic("unreachable code");
}

const volatile struct Env *env;
extern int main(int, char **);

void libmain(int argc, char **argv)
{

	// set env to point at our env structure in envs[].
	env = &envs[ENVX(syscall_getenvid())];//这是为了让libmain函数能够访问到当前进程的env结构体

	// call user main routine
	int result = main(argc, argv);//调用用户的main函数
	// envs[env->env_parent_id].env_flag = result;
	syscall_send (result);
	// exit gracefully
	exit();
}
