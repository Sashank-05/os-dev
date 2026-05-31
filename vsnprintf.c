int vsnprintf(char* str, size_t size, const char* fmt, va_list ap) {
    size_t written = 0;
    while (*fmt && written < size - 1) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd' || *fmt == 'i') {
                long val = va_arg(ap, int);
                char buf[32];
                int i = 0;
                if (val == 0) buf[i++] = '0';
                else {
                    if (val < 0) { if (written < size-1) str[written++] = '-'; val = -val; }
                    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
                }
                while (i > 0 && written < size - 1) str[written++] = buf[--i];
            } else if (*fmt == 's') {
                char* s = va_arg(ap, char*);
                if (!s) s = "(null)";
                while (*s && written < size - 1) str[written++] = *s++;
            } else if (*fmt == 'c') {
                str[written++] = (char)va_arg(ap, int);
            } else if (*fmt == '%') {
                str[written++] = '%';
            }
        } else {
            str[written++] = *fmt;
        }
        fmt++;
    }
    if (written < size) str[written] = '\0';
    return written;
}

int snprintf(char* str, size_t size, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int res = vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return res;
}
