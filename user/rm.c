#include <lib.h>

void usage(void) {
	printf("usage: rm error!\n");
	exit();
}

int main(int argc, char **argv) {
    int r;
    struct Stat st;

    if (argc == 3) {
        if (strcmp(argv[1], "-r") == 0)
        {
            if (((r = stat(argv[2], &st)) < 0)) 
            {
                printf("rm: cannot remove '%s': No such file or directory\n", argv[2]);
                return 0;
            } 
            else 
            {
                remove(argv[2]);
            }
        }
    } 
    else if (argc == 3 && strcmp(argv[1], "-rf") == 0) 
    {
        remove(argv[2]);
    }
    else if (argc == 2)
    {
        r = stat(argv[1], &st);
        if (r < 0) 
        {
            printf("rm: cannot remove '%s': No such file or directory\n", argv[1]);
        } 
        else if (st.st_isdir == 1) 
        {
            printf("rm: cannot remove '%s': Is a directory\n", argv[1]);
        } 
        else 
        {
            remove(argv[1]);
        }
    } 
    else 
    {
        usage();
    }
 	return 0;
}