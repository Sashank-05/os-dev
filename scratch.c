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

int printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
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
    va_end(ap);
    return 0;
}

int puts(const char* s) {
    sys_write(s);
    sys_write("\n");
    return 0;
}
