#pragma once

#include <stddef.h>
#include <stdint.h>

void keyboard_handle_scancode(uint8_t scancode);
size_t keyboard_readline(char* buffer, size_t buffer_size);
int keyboard_poll_event(uint8_t* pressed, uint8_t* key);