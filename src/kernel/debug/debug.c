#include <stdarg.h>
#include <debug/debug.h>
#include <common/stdinfo.h>
#include <drivers/framebuffer.h>


void debug(const char* str, ...){
    va_list args;
    va_start(args, str);
    char buffer[1024];
    vsnprintf(buffer, 1024, str, args);
    serial_print(COM1, buffer);
    va_end(args);
   
}


void log(enum LOG_LEVEL level, const char* str, ...){
    va_list args;
    va_start(args, str);
    char buffer[1024];
    vsnprintf(buffer, 1024, str, args);
    switch (level)
    {
    case INFO:
        printk(GFX_WHITE, "[");
        printk(GFX_GREEN, "INFO");
        printk(GFX_WHITE, "] ");
        break;
    case WARN:
        printk(GFX_WHITE, "[");
        printk(GFX_YELLOW, "WARN");
        printk(GFX_WHITE, "] ");
        break;
    case ERROR:
        printk(GFX_WHITE, "[");
        printk(GFX_RED, "ERROR");
        printk(GFX_WHITE, "] ");
        break;
    case DEBUG:
        printk(GFX_WHITE, "[");
        printk(GFX_CYAN, "DEBUG");
        printk(GFX_WHITE, "] ");
        break;
    default:
        break;
    }
    printk(GFX_WHITE, buffer);
    va_end(args);
}