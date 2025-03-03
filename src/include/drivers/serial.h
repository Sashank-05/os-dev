#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>
#include <stddef.h>
#include <io.h>
#include <stdarg.h>
#include <libk/stdio.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

void serial_init(uint16_t port);
int serial_received(uint16_t port);
char serial_read(uint16_t port);
void serial_write(uint16_t port, char a);
void serial_print(uint16_t port, const char* str, ...);


#endif