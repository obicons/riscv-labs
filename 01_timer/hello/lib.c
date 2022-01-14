#include "lib.h"

void* memcpy(void *restrict dst, const void *restrict src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    for (uint64_t i = 0; i < n; i++) {
	*(d + i) = *(s + i);
    }
    return dst;
}

void* memset(void *b, int c, size_t len) {
    unsigned char *bp = b;
    for (uint64_t i = 0; i < len; i++) {
	bp[i] = c;
    }
    return b;
}
