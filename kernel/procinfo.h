#ifndef PROCINFO_H
#define PROCINFO_H

enum procstate {
    UNUSED,
    USED,
    SLEEPING,
    RUNNABLE,
    RUNNING,
    ZOMBIE,
};

struct procinfo {
    int pid;
    int ppid;
    int state;
    char name[16];
    char pname[16];
};

#define PS_ERR_ADDR  (-1)
#define PS_ERR_LIMIT (-2)

#endif
