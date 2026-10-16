#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"



static int
hexval(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "usage: hexwrite HEXSTR file\n");
        exit(1);
    }

    char *s = argv[1];
    int slen = strlen(s);
    if (slen % 2 != 0) {
        fprintf(2, "hexwrite: odd hex length\n");
        exit(1);
    }

    int nbytes = slen / 2;
    uchar *out = malloc(nbytes);
    if (out == 0) {
        fprintf(2, "hexwrite: malloc failed\n");
        exit(1);
    }

    for (int i = 0; i < nbytes; i++) {
        int hi = hexval(s[2*i]);
        int lo = hexval(s[2*i + 1]);
        if (hi < 0 || lo < 0) {
            fprintf(2, "hexwrite: bad hex char\n");
            free(out);
            exit(1);
        }
        out[i] = (hi << 4) | lo;
    }

    int fd = open(argv[2], O_WRONLY);
    if (fd < 0) {
        fprintf(2, "hexwrite: cannot open %s\n", argv[2]);
        free(out);
        exit(1);
    }

    int off = 0;
    while(off < nbytes) {
        int w = write(fd, out + off, nbytes - off);
        if (w <= 0) {
            fprintf(2, "hexwrite: Write error\n");
            close(fd);
            free(out);
            exit(1);
        }
        off += w;
    }

    close(fd);
    free(out);
    exit(0);
}
