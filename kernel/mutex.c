#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mutex.h"

int
mutexalloc(struct file **f)
{
    struct mutex *m;
    *f = 0;

    if ((*f = filealloc()) == 0) {
        return -1;
    }
    if ((m = (struct mutex *)kalloc()) == 0) {
        fileclose(*f);
        return -1;
    }

    initsleeplock(&m->lock, "mutex");
    (*f)->type     = FD_MUTEX;
    (*f)->readable = 0;
    (*f)->writable = 0;
    (*f)->mtx      = m;

    return 0;
}

void
mutexclose(struct mutex *m)
{
    if (holdingsleep(&m->lock)) {
        releasesleep(&m->lock);
    }
    kfree((char *)m);
}
