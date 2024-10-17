#ifndef kb_h
#define kb_h

#include <util.h>
#include <boot_print.h>
#include <kernel.h>
#include <io.h>
#include <idt.h>
#include <isr.h>

static void kb_callback(registers_t *regs);

void kb_install();

#endif