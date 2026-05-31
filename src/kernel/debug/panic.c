#include <drivers/framebuffer.h>
#include <stdint.h>
#include <debug/panic.h>
#include <reg.h>
#include <stdarg.h>
#include <libk/stdio.h>
#include <debug/debug.h>

void panic(char* string, ...) {
      framebuffer_init(0xFFFFFF);
      va_list args;
      va_start(args, string);
      char msg[1024];
      vsnprintf(msg, sizeof(msg), string, args);
      va_end(args);

      printk(GFX_RED, "\n");
      printk(GFX_RED, "KERNEL PANIC: %s\n", msg);
      debug("KERNEL PANIC: %s\n", msg);
      printk(GFX_RED, "System halted\n");
      debug("System halted\n");

      while (1) {
            __asm__ volatile ("hlt");
      }
}
