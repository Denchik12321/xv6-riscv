#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
expect(const char *what, int got, int want)
{
    if (got == want) {
        printf("OK   %s: %d\n", what, got);
    } else {
        printf("FAIL %s: got %d, want %d\n", what, got, want);
        exit(1);
    }
}

void
main(void)
{
    int status;

    int mtx = mutex();
    expect("mutex() >= 0", mtx >= 0, 1);

    // read/write
    char buf[4];
    expect("read(mutex)",  read(mtx, buf, 4),  -1);
    expect("write(mutex)", write(mtx, "x", 1), -1);

    // lock/unlock
    expect("lock",   mutex_lock(mtx),   0);
    expect("unlock", mutex_unlock(mtx), 0);
    expect("unlock unlocked", mutex_unlock(mtx), -1);

    // close
    expect("close", close(mtx), 0);

    // close своего захваченного мьютекса
    mtx = mutex();
    mutex_lock(mtx);
    expect("close locked", close(mtx), 0);

    // fork делит мьютекс
    mtx = mutex();
    int pid = fork();
    if (pid < 0) {
        printf("fork() failed\n");
        exit(1);
    }
    if (pid == 0) {
        if (mutex_lock(mtx)   != 0) exit(1);
        if (mutex_unlock(mtx) != 0) exit(1);
        if (close(mtx)        != 0) exit(1);
        exit(0);
    }
    close(mtx);
    wait(&status);
    expect("fork shared ref", status, 0);

    // unlock чужим процессом
    mtx = mutex();
    mutex_lock(mtx);
    pid = fork();
    if (pid < 0) {
        printf("fork() failed\n");
        exit(1);
    }
    if (pid == 0) {
        int r = mutex_unlock(mtx);
        exit(r == -1 ? 0 : 1);
    }
    wait(&status);
    expect("cross-proccess unlock", status, 0);
    close(mtx);

    // close чужим процессом
    mtx = mutex();
    mutex_lock(mtx);
    pid = fork();
    if (pid < 0) {
        printf("fork() failed\n");
        exit(1);
    }
    if (pid == 0) {
        int r = close(mtx);
        exit(r == 0 ? 0 : 1);
    }
    wait(&status);
    expect("cross-proccess close", status, 0);
    close(mtx);

    // exit с захваченным мьютексом
    mtx = mutex();
    pid = fork();
    if (pid < 0) {
        printf("fork() failed\n");
        exit(1);
    }
    if (pid == 0) {
        mutex_lock(mtx);
        exit(0);
    }
    wait(&status);
    expect("cross-proccess lock-exit-lock", mutex_lock(mtx), 0); // Если не освобожден, то зависаем
    close(mtx);

    printf("ALL OK\n");
    exit(0);
}
