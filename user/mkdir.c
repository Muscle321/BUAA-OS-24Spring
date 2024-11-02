#include <lib.h>

void usage(void) {
	printf("usage: mkdir error!\n");
	exit();
}

int main(int argc, char **argv) {
    int r;

    if (argc == 3) {
        if ((strcmp(argv[1], "-p") == 0))
            user_create(argv[2], 1);
    } 
    else if (argc == 2)
    {
        r = user_create(argv[1], 1);
        if (r == -E_NOT_DIR) {
            printf("mkdir: cannot create directory '%s': No such file or directory\n", argv[1]);
        } else if (r == -E_FILE_EXISTS) {
            printf("mkdir: cannot create directory '%s': File exists\n", argv[1]);
        }
    } 
    else 
    {
        usage();
    }
 	return 0;
}
