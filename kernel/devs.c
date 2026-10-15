#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "devs.h"

static struct {
    struct spinlock lock;
    uint64 urandom_state;
    uint64 nullstat_bytes;
} pseudo;

static inline uint64
lcg_next(uint64 *s)
{
    *s = (*s) * 6364136223846793005ULL + 1442695040888963407ULL;
    return *s;
}

static int
pseudo_read(int user_dst, uint64 dst, int n, int minor)
{
    char buf[128];
    int total = 0;

    switch(minor)
    {
        case MINOR_NULL:
        {
            return 0;
        }

        case MINOR_ZERO:
        {
            memset(buf, 0, sizeof(buf));
            while(total < n) {
                int chunk = n - total;

                if (chunk > (int)sizeof(buf)) {
                    chunk = sizeof(buf);
                }
                if (either_copyout(user_dst, dst + total, buf, chunk) < 0) {
                    return -1;
                }

                total += chunk;
            }
            return n;
        }

        case MINOR_URANDOM:
        {
            while(total < n) {
                int chunk = n - total;
                
                if (chunk > (int)sizeof(buf)) {
                    chunk = sizeof(buf);
                }

                acquire(&pseudo.lock);
                for (int i = 0; i < chunk; i++) {
                    buf[i] = (char)(lcg_next(&pseudo.urandom_state) >> 33);
                }
                release(&pseudo.lock);

                if (either_copyout(user_dst, dst + total, buf, chunk) < 0) {
                    return -1;
                }

                total += chunk;
            }
            return n;
        }

        case MINOR_NULLSTAT:
        {
            if (n != (int)sizeof(uint64)) {
                return -1;
            }

            acquire(&pseudo.lock);
            uint64 v = pseudo.nullstat_bytes;
            release(&pseudo.lock);

            memmove(buf, &v, sizeof(uint64));
            if (either_copyout(user_dst, dst, buf, sizeof(uint64)) < 0) {
                return -1;
            }

            return sizeof(uint64);
        }
    }

    return -1;
}

static int
pseudo_write(int user_src, uint64 src, int n, int minor)
{
    switch(minor)
    {
        case MINOR_NULL:
        {
            return n;
        }

        case MINOR_ZERO:
        {
            return -1;
        }

        case MINOR_URANDOM:
        {
            if (n != (int)sizeof(uint64)) {
                return -1;
            }

            uint64 seed;
            if (either_copyin(&seed, user_src, src, sizeof(seed)) < 0) {
                return -1;
            }

            acquire(&pseudo.lock);
            pseudo.urandom_state = seed;
            release(&pseudo.lock);

            return sizeof(uint64);
        }

        case MINOR_NULLSTAT:
        {
            acquire(&pseudo.lock);
            pseudo.nullstat_bytes += (uint64)n;
            release(&pseudo.lock);

            return n;
        }
    }

    return -1;
}

void
devs_init(void)
{
    initlock(&pseudo.lock, "pseudo");
    pseudo.urandom_state = 0x123456789abcdef0ULL;
    pseudo.nullstat_bytes = 0;

    devsw[DEV_PSEUDO].read  = pseudo_read;
    devsw[DEV_PSEUDO].write = pseudo_write;
}
