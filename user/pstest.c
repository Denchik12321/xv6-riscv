#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
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

int
main(void)
{
    // NULL -> число процессов
    int n = ps_listinfo(0, 0);
    if (n <= 0) {
        printf("FAIL count call returned %d\n", n);
        exit(1);
    }
    printf("OK   count = %d\n", n);

    // Буффер ровно по размеру
    struct procinfo *buf = malloc(n * sizeof(struct procinfo));
    expect("exact value", ps_listinfo(buf, n), n);
    free(buf);

    // Буффер больше размера
    buf = malloc((n + 10) * sizeof(struct procinfo));
    expect("bigger size", ps_listinfo(buf, n + 10), n);
    free(buf);

    // Слишком маленький буфер
    int pid = fork();
    if (pid == 0) {
        pause(20);
        exit(0);
    }
    buf = malloc(sizeof(struct procinfo));
    expect("small buffer", ps_listinfo(buf, 1), PS_ERR_LIMIT);
    free(buf);

    // Плохой адрес
    struct procinfo *bad = (struct procinfo *)0xdeaddead;
    expect("bad address", ps_listinfo(bad, 10), PS_ERR_ADDR);

    int status;
    wait(&status);
    
    printf("ALL OK\n");
    exit(0);
}