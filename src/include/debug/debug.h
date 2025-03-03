#pragma once

#include <stddef.h>
#include <stdint.h>

#include <drivers/serial.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <libk/stdlib.h>
#include <common/stdinfo.h>


void debug(const char* str, ...);
void log(enum LOG_LEVEL level, const char* str, ...);

