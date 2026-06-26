#include <stddef.h>

#include "common/types.h"

void *memset(void *dest, int value, size_t length)
{
    uint8_t *ptr = (uint8_t *)dest;

    while (length-- != 0U) {
        *ptr++ = (uint8_t)value;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t length)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    while (length-- != 0U) {
        *d++ = *s++;
    }
    return dest;
}
