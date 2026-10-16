#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

static char *states[] = {
    [UNUSED]   "unused",
    [USED]     "used",
    [SLEEPING] "sleeping",
    [RUNNABLE] "runnable",
    [RUNNING]  "running",
    [ZOMBIE]   "zombie",
};

int
main(int argc, char *argv[])
{
    int n = ps_listinfo(0, 0);
    if (n < 0) {
        fprintf(2, "ps: ps_listinfo(NULL) returned %d\n", n);
        exit(1);
    }

    int cap = n + 8;
    struct procinfo *list = malloc(cap * sizeof(struct procinfo));
    if (list == 0) {
        fprintf(2, "ps: malloc failed\n");
        exit(1);
    }

    int m = ps_listinfo(list, cap);
    if (m < 0) {
        fprintf(2, "ps: ps_listinfo returned %d\n", m);
        free(list);
        exit(1);
    }

    printf("pid  ppid  state  name  pname\n");
    for (int i = 0; i < m; i++) {
        int st = list[i].state;
        char *s = (st >= 0 && st < 6) ? states[st] : "???";
        printf("%d %d %s %s %s\n", list[i].pid, list[i].ppid, s, list[i].name, list[i].pname);
    }

    free(list);
    exit(0);
}
