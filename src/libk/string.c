#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, uint64_t c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

#include <stdint.h>

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    // If src > dest, copy forward
    if (psrc > pdest) {
        // We can use a larger chunk copy here
        while (n >= 8) {
            *(uint64_t *)pdest = *(uint64_t *)psrc;  // Copy 8 bytes at a time
            pdest += 8;
            psrc += 8;
            n -= 8;
        }
        // Handle remaining bytes (if n is not a multiple of 8)
        while (n--) {
            *pdest++ = *psrc++;
        }
    }
    // If src < dest, copy backward
    else if (psrc < pdest) {
        pdest += n;      // Move to the end of the destination
        psrc += n;       // Move to the end of the source
        while (n >= 8) {
            pdest -= 8;
            psrc -= 8;
            *(uint64_t *)pdest = *(uint64_t *)psrc;  // Copy 8 bytes backward
            n -= 8;
        }
        // Handle remaining bytes (if n is not a multiple of 8)
        while (n--) {
            *(--pdest) = *(--psrc);
        }
    }

    return dest;
}


int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}


int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return n ? *(const unsigned char *)s1 - *(const unsigned char *)s2 : 0;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

char *strcat(char *dest, const char *src) {
    strcpy(dest + strlen(dest), src);
    return dest;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

char *strrchr(const char *s, int c) {
    char *last = NULL;
    while (*s) {
        if (*s == c) {
            last = (char *)s;
        }
        s++;
    }
    return last;
}

char *strstr(const char *haystack, const char *needle) {
    size_t needle_len = strlen(needle);
    if (!needle_len) {
        return (char *)haystack;
    }
    while (*haystack) {
        if (*haystack == *needle) {
            if (!memcmp(haystack, needle, needle_len)) {
                return (char *)haystack;
            }
        }
        haystack++;
    }
    return NULL;
}

char *strtok(char *str, const char *delim) {
    static char *last = NULL;
    if (str) {
        last = str;
    } else {
        if (!last) {
            return NULL;
        }
        str = last;
    }
    while (*last) {
        if (strchr(delim, *last)) {
            *last++ = '\0';
            return str;
        }
        last++;
    }
    return str;
}

/*

char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    if (p) {
        memcpy(p, s, len);
    }
    return p;
}
*/
