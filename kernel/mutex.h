#ifndef MUTEX_H
#define MUTEX_H

struct file;

struct mutex {
    struct sleeplock lock;
};

int mutexalloc(struct file **f);
void mutexclose(struct mutex *m);

#endif
