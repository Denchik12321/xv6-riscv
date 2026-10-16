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
    uint64 state;
} urandom_state;

static struct {
    struct spinlock lock;
    uint64 bytes;
} nullstat_bytes;

static char zero_buf[PGSIZE];

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
            while(total < n) {
                int chunk = n - total;

                if (chunk > (int)sizeof(zero_buf)) {
                    chunk = sizeof(zero_buf);
                }
                if (either_copyout(user_dst, dst + total, zero_buf, chunk) < 0) {
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

                acquire(&urandom_state.lock);
                for (int i = 0; i < chunk; i++) {
                    buf[i] = (char)(lcg_next(&urandom_state.state) >> 33);
                }
                release(&urandom_state.lock);

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

            acquire(&nullstat_bytes.lock);
            uint64 v = nullstat_bytes.bytes;
            release(&nullstat_bytes.lock);

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

            acquire(&urandom_state.lock);
            urandom_state.state = seed;
            release(&urandom_state.lock);

            return sizeof(uint64);
        }

        case MINOR_NULLSTAT:
        {
            acquire(&nullstat_bytes.lock);
            nullstat_bytes.bytes += (uint64)n;
            release(&nullstat_bytes.lock);

            return n;
        }
    }

    return -1;
}

void
devs_init(void)
{
    initlock(&urandom_state.lock, "urandom");
    initlock(&nullstat_bytes.lock, "nullstat");

    urandom_state.state = 0x123456789abcdef0ULL;
    nullstat_bytes.bytes = 0;

    devsw[DEV_PSEUDO].read  = pseudo_read;
    devsw[DEV_PSEUDO].write = pseudo_write;
}
