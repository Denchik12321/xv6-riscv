#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "usage: hexdump N file\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    int fd = open(argv[2], O_RDONLY);
    if (fd < 0) {
        fprintf(2, "hexdump: cannot open %s\n", argv[2]);
        exit(1);
    }

    uchar buf[64];
    int left = n;
    int first = 1;

    while (left > 0) {
        int chunk = left < (int)sizeof(buf) ? left : (int)sizeof(buf);
        int r = read(fd, buf, chunk);

        if (r < 0) {
            fprintf(2, "hexdump: read error\n");
            exit(1);
        }
        if (r == 0) break;

        for (int i = 0; i < r; i++) {
            if (!first) {
                printf(" ");
            }

            char *hex = "0123456789ABCDEF";

            printf("%c%c", hex[(buf[i] >> 4) & 0x0F], hex[buf[i] & 0x0F]);
            first = 0;
        }
        left -= r;
    }

    printf("\n");
    close(fd);
    exit(0);
}
