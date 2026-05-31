#include <drivers/keyboard.h>

#include <asm/asmcalls.h>
#include <libk/string.h>
#include <drivers/framebuffer.h>

#define KEYBOARD_BUFFER_SIZE 1024

static volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile size_t keyboard_head = 0;
static volatile size_t keyboard_tail = 0;
static volatile uint8_t left_shift = 0;
static volatile uint8_t right_shift = 0;
static volatile uint8_t e0_prefix = 0;

struct keyboard_event {
    uint8_t pressed;
    uint8_t key;
};

static volatile struct keyboard_event event_buffer[KEYBOARD_BUFFER_SIZE];
static volatile size_t event_head = 0;
static volatile size_t event_tail = 0;

#define KEY_EXT_UP    0x80
#define KEY_EXT_DOWN  0x81
#define KEY_EXT_LEFT  0x82
#define KEY_EXT_RIGHT 0x83

static const char scancode_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
};

static const char scancode_shift_map[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0,
};

static void keyboard_push_char(char c) {
    size_t next_head = (keyboard_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_head == keyboard_tail) {
        return;
    }

    keyboard_buffer[keyboard_head] = c;
    keyboard_head = next_head;
}

static void keyboard_push_event(uint8_t pressed, uint8_t key) {
    size_t next_head = (event_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_head == event_tail) {
        return;
    }

    event_buffer[event_head].pressed = pressed;
    event_buffer[event_head].key = key;
    event_head = next_head;
}

static int keyboard_pop_char(char* out) {
    if (keyboard_tail == keyboard_head) {
        return 0;
    }

    *out = keyboard_buffer[keyboard_tail];
    keyboard_tail = (keyboard_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return 1;
}

void keyboard_handle_scancode(uint8_t scancode) {
    if (scancode == 0xE0) {
        e0_prefix = 1;
        return;
    }

    if (e0_prefix) {
        uint8_t released = (scancode & 0x80) ? 1 : 0;
        uint8_t code = (uint8_t)(scancode & 0x7F);
        uint8_t ext_key = 0;

        switch (code) {
        case 0x48:
            ext_key = KEY_EXT_UP;
            break;
        case 0x50:
            ext_key = KEY_EXT_DOWN;
            break;
        case 0x4B:
            ext_key = KEY_EXT_LEFT;
            break;
        case 0x4D:
            ext_key = KEY_EXT_RIGHT;
            break;
        default:
            break;
        }

        if (ext_key) {
            keyboard_push_event(released ? 0 : 1, ext_key);
        }

        e0_prefix = 0;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) {
        if (scancode == 0x2A) {
            left_shift = 1;
        }
        else {
            right_shift = 1;
        }
        return;
    }

    if (scancode == 0xAA || scancode == 0xB6) {
        if (scancode == 0xAA) {
            left_shift = 0;
        }
        else {
            right_shift = 0;
        }
        return;
    }

    if (scancode & 0x80) {
        uint8_t released_scancode = (uint8_t)(scancode & 0x7F);
        char released = (left_shift || right_shift) ? scancode_shift_map[released_scancode] : scancode_map[released_scancode];
        if (released) {
            keyboard_push_event(0, released);
        }
        return;
    }

    char c = (left_shift || right_shift) ? scancode_shift_map[scancode] : scancode_map[scancode];
    if (!c) {
        return;
    }

    keyboard_push_event(1, c);
    keyboard_push_char(c);
}

int keyboard_poll_event(uint8_t* pressed, uint8_t* key) {
    if (!pressed || !key) {
        return 0;
    }

    if (event_tail == event_head) {
        return 0;
    }

    *pressed = event_buffer[event_tail].pressed;
    *key = event_buffer[event_tail].key;
    event_tail = (event_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return 1;
}

size_t keyboard_readline(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return 0;
    }

    size_t written = 0;
    for (;;) {
        char c;
        while (!keyboard_pop_char(&c)) {
            __asm__ volatile ("hlt");
        }

        if (c == '\b') {
            if (written > 0) {
                written--;
                printk(GFX_WHITE, "\b");
            }
            continue;
        }

        if (c == '\n') {
            buffer[written] = '\0';
            printk(GFX_WHITE, "\n");
            return written;
        }

        if (written + 1 < buffer_size) {
            buffer[written++] = c;
            printk(GFX_WHITE, "%c", c);
        }
    }
}
