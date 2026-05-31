// Minimal libc for doomgeneric

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

int printf(const char* fmt, ...);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct {
    char path[128];
    long offset;
    long size;
} FILE;

// Strings (first, so they're available for memory funcs)
void* memcpy(void* dst, const void* src, size_t n) {
    unsigned char* d = dst;
    const unsigned char* s = src;
    while (n--) *d++ = *s++;
    return dst;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) *p++ = c;
    return s;
}

// Memory
static char heap[256*1024*1024] __attribute__((aligned(16)));  // 256MB heap
static size_t heap_pos = 0;

void* malloc(size_t size) {
    heap_pos = (heap_pos + 15) & ~15; // Align to 16 bytes for SSE
    if (heap_pos + size > sizeof(heap)) {
        printf("malloc(%u) FAILED (pos=%u, total=%u)\n", (unsigned int)size, (unsigned int)heap_pos, (unsigned int)sizeof(heap));
        return NULL;
    }
    void* ptr = heap + heap_pos;
    heap_pos += size;
    return ptr;
}

void free(void* ptr) { }  // No-op for now

void* calloc(size_t num, size_t size) {
    void* ptr = malloc(num * size);
    if (ptr) memset(ptr, 0, num * size);
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    void* new = malloc(size);
    if (new && ptr) memcpy(new, ptr, size);
    return new;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* a = s1, *b = s2;
    while (n--) {
        if (*a != *b) return *a - *b;
        a++; b++;
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}

char* strcpy(char* dst, const char* src) {
    char* d = dst;
    while ((*d++ = *src++));
    return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
    char* d = dst;
    while (n > 0 && *src) {
        *d++ = *src++;
        n--;
    }
    while (n > 0) {
        *d++ = '\0';
        n--;
    }
    return dst;
}

int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char* a, const char* b, size_t n) {
    if (n == 0) return 0;
    while (n > 1 && *a && *a == *b) {
        a++;
        b++;
        n--;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strcasecmp(const char* a, const char* b) {
    while (*a || *b) {
        char ca = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a;
        char cb = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return 0;
}

int strncasecmp(const char* a, const char* b, size_t n) {
    while (n--) {
        if (!*a && !*b) return 0;
        char ca = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a;
        char cb = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return 0;
}

char* strchr(const char* s, int c) {
    while (*s && *s != c) s++;
    return *s == c ? (char*)s : NULL;
}

char* strrchr(const char* s, int c) {
    const char* r = NULL;
    while (*s) {
        if (*s == c) r = s;
        s++;
    }
    return (char*)r;
}

char* strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* d = malloc(len);
    if (d) strcpy(d, s);
    return d;
}

// Character functions
int toupper(int c) {
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

int tolower(int c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

int isdigit(int c) { return c >= '0' && c <= '9'; }
int isalpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

// Conversion
int atoi(const char* str) {
    int val = 0, sign = 1;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;
    while (*str >= '0' && *str <= '9') {
        val = val * 10 + (*str - '0');
        str++;
    }
    return val * sign;
}

long atol(const char* str) { return atoi(str); }

double atof(const char* str) {
    double val = 0.0, sign = 1.0, exp = 1.0;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == '-') { sign = -1.0; str++; }
    else if (*str == '+') str++;
    while (*str >= '0' && *str <= '9') {
        val = val * 10.0 + (*str - '0');
        str++;
    }
    if (*str == '.') {
        str++;
        exp = 0.1;
        while (*str >= '0' && *str <= '9') {
            val += exp * (*str - '0');
            exp *= 0.1;
            str++;
        }
    }
    return val * sign;
}

// Math
double fabs(double x) { return x < 0 ? -x : x; }
int abs(int x) { return x < 0 ? -x : x; }
long labs(long x) { return x < 0 ? -x : x; }

int vprintf(const char* fmt, va_list ap);
int vsnprintf(char* str, size_t size, const char* fmt, va_list ap);
int snprintf(char* str, size_t size, const char* fmt, ...);

// File I/O stubs
static long syscall3(long number, long arg1, long arg2, long arg3) {
    register long rax asm("rax") = number;
    register long rdi asm("rdi") = arg1;
    register long rsi asm("rsi") = arg2;
    register long rdx asm("rdx") = arg3;
    asm volatile("int $0x80" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r8", "r9", "r10", "r11", "memory");
    return rax;
}

static long syscall4(long number, long arg1, long arg2, long arg3, long arg4) {
    register long rax asm("rax") = number;
    register long rdi asm("rdi") = arg1;
    register long rsi asm("rsi") = arg2;
    register long rdx asm("rdx") = arg3;
    register long r10 asm("r10") = arg4;
    asm volatile("int $0x80" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10) : "rcx", "r8", "r9", "r11", "memory");
    return rax;
}

static FILE open_files[16];
static int open_file_count = 0;

FILE* fopen(const char* name, const char* mode) {
    long size = syscall3(10, (long)name, 0, 0); // SYS_filesize
    if (size < 0) return NULL;

    if (open_file_count >= 16) return NULL;
    FILE* f = &open_files[open_file_count++];
    strncpy(f->path, name, 127);
    f->offset = 0;
    f->size = size;
    return f;
}

int fclose(FILE* f) {
    return 0;
}

int fseek(FILE* f, long off, int whence) {
    if (!f) return -1;
    if (whence == SEEK_SET) f->offset = off;
    else if (whence == SEEK_CUR) f->offset += off;
    else if (whence == SEEK_END) f->offset = f->size + off;
    
    if (f->offset < 0) f->offset = 0;
    if (f->offset > f->size) f->offset = f->size;
    return 0;
}

size_t fread(void* buf, size_t size, size_t count, FILE* f) {
    if (!f) return 0;
    long total = size * count;
    if (total <= 0) return 0;
    
    long read = syscall4(6, (long)f->path, (long)buf, (long)total, (long)f->offset); // SYS_readfile
    if (read < 0) return 0;
    
    f->offset += read;
    return read / size;
}

int fgetc(FILE* f) {
    unsigned char c;
    if (fread(&c, 1, 1, f) != 1) return -1;
    return c;
}

long ftell(FILE* f) {
    return f ? f->offset : -1;
}

size_t fwrite(const void* buf, size_t size, size_t count, FILE* f) { return 0; }
int fflush(FILE* f) { return 0; }
int putchar(int c) { return 0; }
int system(const char* cmd) { return -1; }
int sscanf(const char* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int count = 0;
    while (*fmt && *str) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd' || *fmt == 'i') {
                int* p = va_arg(ap, int*);
                int val = 0;
                while (*str == ' ' || *str == '\t') str++;
                int neg = 0;
                if (*str == '-') { neg = 1; str++; }
                
                if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
                    str += 2;
                    while (1) {
                        if (*str >= '0' && *str <= '9') val = val * 16 + (*str - '0');
                        else if (*str >= 'a' && *str <= 'f') val = val * 16 + (*str - 'a' + 10);
                        else if (*str >= 'A' && *str <= 'F') val = val * 16 + (*str - 'A' + 10);
                        else break;
                        str++;
                    }
                } else {
                    while (*str >= '0' && *str <= '9') {
                        val = val * 10 + (*str - '0');
                        str++;
                    }
                }
                *p = neg ? -val : val;
                count++;
            } else if (*fmt == 'x') {
                int* p = va_arg(ap, int*);
                unsigned int val = 0;
                while (*str == ' ' || *str == '\t') str++;
                if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) str += 2;
                while (1) {
                    if (*str >= '0' && *str <= '9') val = val * 16 + (*str - '0');
                    else if (*str >= 'a' && *str <= 'f') val = val * 16 + (*str - 'a' + 10);
                    else if (*str >= 'A' && *str <= 'F') val = val * 16 + (*str - 'A' + 10);
                    else break;
                    str++;
                }
                *p = (int)val;
                count++;
            } else if (*fmt == 's') {
                char* p = va_arg(ap, char*);
                while (*str == ' ' || *str == '\t') str++;
                while (*str && *str != ' ' && *str != '\t' && *str != '\n') {
                    *p++ = *str++;
                }
                *p = '\0';
                count++;
            }
        } else if (*fmt == *str) {
            str++;
        } else if (*fmt == ' ' && (*str == ' ' || *str == '\t' || *str == '\n')) {
            while (*str == ' ' || *str == '\t' || *str == '\n') str++;
        } else {
            break;
        }
        fmt++;
    }
    va_end(ap);
    return count;
}
int __isoc23_sscanf(const char* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int res = sscanf(str, fmt, ap); // Hacky
    va_end(ap);
    return res;
}
char* strstr(const char* h, const char* n) {
    while (*h) {
        const char* a = h, *b = n;
        while (*a && *b && *a == *b) { a++; b++; }
        if (!*b) return (char*)h;
        h++;
    }
    return NULL;
}
void* memmove(void* d, const void* s, size_t n) {
    return memcpy(d, s, n);
}
int mkdir(const char* p, int m) { return 0; }
int remove(const char* f) { return 0; }
int rename(const char* o, const char* n) { return 0; }
int* __errno_location(void) { static int e = 0; return &e; }
int* __ctype_b_loc(void) { static int c = 0; return &c; }

// Networking stubs
int net_client_connected = 0;
int drone = 0;

int fprintf(FILE* f, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int res = vprintf(fmt, ap);
    va_end(ap);
    return res;
}

int vfprintf(FILE* f, const char* fmt, va_list ap) {
    return vprintf(fmt, ap);
}
int sprintf(char* str, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int res = vsnprintf(str, 4096, fmt, ap);
    va_end(ap);
    return res;
}

int vsnprintf(char* str, size_t size, const char* fmt, va_list ap) {
    if (size == 0) return 0;
    size_t written = 0;
    while (*fmt && written < size - 1) {
        if (*fmt == '%') {
            fmt++;
            int zero_pad = 0;
            int width = 0;
            if (*fmt == '0') {
                zero_pad = 1;
                fmt++;
            }
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            if (*fmt == '.') {
                fmt++;
                zero_pad = 1; // For integers, %.3d is like %03d
                width = 0;
                while (*fmt >= '0' && *fmt <= '9') {
                    width = width * 10 + (*fmt - '0');
                    fmt++;
                }
            }
            
            if (*fmt == 'd' || *fmt == 'i' || *fmt == 'u') {
                unsigned long val;
                int neg = 0;
                if (*fmt == 'u') {
                    val = va_arg(ap, unsigned int);
                } else {
                    long sval = va_arg(ap, int);
                    if (sval < 0) { neg = 1; val = -sval; }
                    else val = sval;
                }
                
                char buf[32];
                int i = 0;
                if (val == 0) buf[i++] = '0';
                else {
                    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
                }
                
                if (neg && written < size - 1) str[written++] = '-';
                
                int pad = width - i - neg;
                while (pad > 0 && zero_pad && written < size - 1) {
                    str[written++] = '0';
                    pad--;
                }
                while (pad > 0 && !zero_pad && written < size - 1) {
                    str[written++] = ' ';
                    pad--;
                }
                while (i > 0 && written < size - 1) str[written++] = buf[--i];
            } else if (*fmt == 'x' || *fmt == 'X') {
                unsigned long val = va_arg(ap, unsigned int);
                char buf[32];
                int i = 0;
                if (val == 0) buf[i++] = '0';
                else {
                    while (val > 0) {
                        int rem = val % 16;
                        buf[i++] = rem < 10 ? '0' + rem : (*fmt == 'x' ? 'a' : 'A') + (rem - 10);
                        val /= 16;
                    }
                }
                int pad = width - i;
                while (pad > 0 && zero_pad && written < size - 1) {
                    str[written++] = '0';
                    pad--;
                }
                while (i > 0 && written < size - 1) str[written++] = buf[--i];
            } else if (*fmt == 's') {
                char* s = va_arg(ap, char*);
                if (!s) s = "(null)";
                while (*s && written < size - 1) str[written++] = *s++;
            } else if (*fmt == 'c') {
                if (written < size - 1) str[written++] = (char)va_arg(ap, int);
            } else if (*fmt == '%') {
                if (written < size - 1) str[written++] = '%';
            }
        } else {
            if (written < size - 1) str[written++] = *fmt;
        }
        fmt++;
    }
    str[written] = '\0';
    return written;
}

int snprintf(char* str, size_t size, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int res = vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return res;
}

// I/O stubs

FILE* stderr = (FILE*)0;
FILE* stdout = (FILE*)0;
FILE* stdin = (FILE*)0;

// Exit
void exit(int code) {
    long rax = 1; // SYS_exit
    long rdi = code;
    asm volatile("int $0x80" : "+r"(rax) : "r"(rdi) : "rcx", "r8", "r9", "r10", "r11", "memory");
    while(1);
}

void abort(void) {
    exit(1);
}
#include <stdarg.h>

void sys_write(const char* text) {
    long rax = 0; // SYS_write
    long rdi = (long)text;
    long rsi = 0;
    while (text[rsi]) rsi++;
    asm volatile("int $0x80" : "+r"(rax) : "D"(rdi), "S"(rsi), "d"(0) : "rcx", "r11", "memory");
}

void print_number(long value) {
    char buffer[32];
    int index = 0;
    if (value == 0) { sys_write("0"); return; }
    if (value < 0) { sys_write("-"); value = -value; }
    while (value > 0) {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    }
    while (index > 0) {
        char c[2] = {buffer[--index], 0};
        sys_write(c);
    }
}

void print_hex(unsigned long value) {
    char buffer[32];
    int index = 0;
    if (value == 0) { sys_write("0"); return; }
    while (value > 0) {
        int rem = value % 16;
        buffer[index++] = rem < 10 ? '0' + rem : 'a' + (rem - 10);
        value /= 16;
    }
    while (index > 0) {
        char c[2] = {buffer[--index], 0};
        sys_write(c);
    }
}

int vprintf(const char* fmt, va_list ap) {
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd' || *fmt == 'i') print_number(va_arg(ap, int));
            else if (*fmt == 'u') print_number(va_arg(ap, unsigned int));
            else if (*fmt == 'x' || *fmt == 'X') print_hex(va_arg(ap, unsigned int));
            else if (*fmt == 'p') { sys_write("0x"); print_hex((unsigned long)va_arg(ap, void*)); }
            else if (*fmt == 's') {
                char* s = va_arg(ap, char*);
                sys_write(s ? s : "(null)");
            }
            else if (*fmt == 'c') {
                char c[2] = {(char)va_arg(ap, int), 0};
                sys_write(c);
            }
            else if (*fmt == '%') sys_write("%");
        } else {
            char c[2] = {*fmt, 0};
            sys_write(c);
        }
        fmt++;
    }
    return 0;
}

int printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int res = vprintf(fmt, ap);
    va_end(ap);
    return res;
}

int puts(const char* s) {
    sys_write(s);
    sys_write("\n");
    return 0;
}
