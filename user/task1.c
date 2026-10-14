#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        pause(100);
        exit(1);
    }
    printf("parent pid=%d, child pid=%d\n", getpid(), pid);

    int b = 0;
    if (argc > 1 && argv[1][0] == 'b') {
        kill(pid);
        b = 1;
    }

    int status;
    int wpid = wait(&status);
    printf("waited pid=%d, exit code=%d (killed=%d)\n", wpid, status, b);
    exit(0);
}

// Ответ на вопрос, на который должен был ответить в e-mkn.ru:
// Если запустить (a) и "вручную" убить ребенка через kill, то exit code = -1
/* 
    $ task1 &
    pare$n t pid=6, child pid=7
    kill 7
    $wa ited pid=7, exit code=-1 (killed=0)
*/
