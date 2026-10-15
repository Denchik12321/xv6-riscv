#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
run(int mtx, int argc, char *argv[])
{
    int pid = getpid();
    for (int i = 1; i < argc; i++) {
        for (char* p = argv[i]; *p; ++p) {
            if (mtx >= 0) {
                if (mutex_lock(mtx) < 0) {
                    printf("lock failed\n");
                    exit(1);
                }
            }
            printf("%d: arg %d, char '%c'\n", pid, i, *p);
            if (mtx >= 0) {
                if (mutex_unlock(mtx) < 0) {
                    printf("unlock failed\n");
                    exit(1);
                }
            }
        }
    }
}

int
main(int argc, char *argv[])
{
    int use_mutex = 1;
    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        use_mutex = 0;
        argv++; argc--;
    }

    int mtx = -1;
    if (use_mutex) {
        mtx = mutex();
        if (mtx < 0) {
            printf("mutex() failed\n");
            exit(1);
        }
    }

    int pid = fork();
    if (pid < 0) { 
        printf("fork() failed\n");
        exit(1);
    }
    run(mtx, argc, argv);
    if (pid == 0) {
        exit(0);
    }

    int status;
    wait(&status);

    if (mtx >= 0) {
        close(mtx);
    }
    exit(0);
}
