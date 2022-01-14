#ifndef LIB_H
#define LIB_H

#include <stddef.h>
#include <stdint.h>

// Copies n bytes from src to dst.
void *memcpy(void *restrict dst, const void *restrict src, size_t n);

// Sets len bytes of b to c.
void* memset(void *b, int c, size_t len);

#endif
