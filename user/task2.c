#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    int fds[2];
    if (pipe(fds) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        if (close(0) < 0) {
            fprintf(2, "close(0) failed\n");
            exit(1);
        }
        if (dup(fds[0]) < 0) {
            fprintf(2, "dup failed\n");
            exit(1);
        }
        if (close(fds[0]) < 0) {
            fprintf(2, "close(fds[0]) failed\n");
            exit(1);
        }
        if (close(fds[1]) < 0) {
            fprintf(2, "close(fds[1]) failed\n");
            exit(1);
        }
    
        char* wcargv[] = { "/wc", 0 };
        exec("/wc", wcargv);
        fprintf(2, "exec /wc failed\n");
        exit(1);
    }
    
    if (close(fds[0]) < 0) {
        fprintf(2, "close(fds[0]) failed\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        int n = strlen(argv[i]);
        int off = 0;
        
        while(off < n) {
            int w = write(fds[1], argv[i] + off, n - off);
            if (w <= 0) {
                fprintf(2, "write failed\n");
                exit(1);
            }
            off += w;
        }

        if (write(fds[1], "\n", 1) <= 0) {
            fprintf(2, "write failed\n");
            exit(1);
        }
    }

    if (close(fds[1]) < 0) {
        fprintf(2, "close(fds[1]) failed\n");
        exit(1);
    }
    
    int status;
    wait(&status);
    exit(0);
}
