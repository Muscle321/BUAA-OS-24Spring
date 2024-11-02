#include <lib.h>

void usage(void) {
	printf("usage: touch error!");
	exit();
}
/*char *my_strrchr(const char *s, int c) {
    char *last = NULL;
    while (*s) {
        if (*s == c) {
            last = (char *)s;
        }
        s++;
    }
    return last;
}

char *my_strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}


int touch(const char *path)
{
    struct Env *env;
    env = envs + ENVX(syscall_getenvid());
    char tmp[1024];
    int i = 0;
    while ((env->r_path)[i] != 0)
    {
        tmp[i] = (env->r_path)[i];
        i++;
    }
    tmp[i] = 0;
    int len1 = strlen(tmp);
    int len2 = strlen(path);
    if (len1 == 1)
    {
        strcpy(tmp + 1, path);
    }
    else
    {
        tmp[len1] = '/';
        strcpy(tmp + len1 + 1, path);
        tmp[len1 + 1 + len2] = '\0';
    }
    int r;
    struct Stat st;
    char *last_slash;
    char parent_dir[MAXPATHLEN];

    // Check if the parent directory exists
    last_slash = my_strrchr(path, '/');
    if (last_slash) {
        my_strncpy(parent_dir, tmp, last_slash - tmp);
        parent_dir[last_slash - tmp] = '\0';
        if (((r = stat(parent_dir, &st)) < 0) || !st.st_isdir) {
            printf("touch: cannot touch '%s': No such file or directory\n", path);
            return 0;
        }
    }

    // Check if the file already exists
    if (stat(tmp, &st) == 0) {
        // File exists, do nothing
        return 0;
    }

    if ((r = create(tmp)) < 0)
    {
        user_panic("can't create %s: %d", path, r);
    } else {
        user_panic("Successfully!");
    }
    return r;
}*/

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