#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int fds[2];
    if (pipe(fds) < 0) {
        perror("pipe");
        exit(1);
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        if (close(0) < 0) {
            perror("close(0)");
            exit(1);
        }
        if (dup(fds[0]) < 0) {
            perror("dup(fds[0])");
            exit(1);
        }
        if (close(fds[0]) < 0) {
            perror("close(fds[0])");
            exit(1);
        }
        if (close(fds[1]) < 0) {
            perror("close(fds[1])");
            exit(1);
        }

        char buf[4096];
        ssize_t n;
        while ((n = read(0, buf, sizeof(buf))) > 0) {
            ssize_t off = 0;
            while (off < n) {
                ssize_t w = write(1, buf + off, n - off);
                if (w <= 0) {
                    perror("write");
                    exit(1);
                }
                off += w;
            }
        }

        if (n < 0) {
            perror("read");
            exit(1);
        }
        exit(0);
    }

    if (close(fds[0])) {
        perror("close(fds[0])");
        exit(1);
    }

    for (int i = 1; i < argc; ++i) {
        size_t n = strlen(argv[i]);
        size_t off = 0;

        while (off < n) {
            ssize_t w = write(fds[1], argv[i] + off, n - off);
            if (w <= 0) {
                perror("write");
                exit(1);
            }
            off += w;
        }

        if (write(fds[1], "\n", 1) != 1) {
            perror("write");
            exit(1);
        }
    }

    if (close(fds[1])) {
        perror("close(fds[1])");
        exit(1);
    }

    int status;
    wait(&status);
    return 0;
}
