#ifndef GLOBAL

#define GLOBAL

#define elif else if

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// boot
extern struct limine_framebuffer *framebuffer;


// gfx fallback

typedef struct {
    unsigned short magic;  // Magic number (0x0436 for PSF1)
    unsigned char mode;    // Font mode (bit 0 = 512 characters if set, otherwise 256)
    unsigned char charsize; // Character height in pixels (typically 16 or 8)
} psf1_header;

extern unsigned char _binary_font_psf_start[];
extern int cursor_x;
extern int cursor_y;
void draw_pixel(unsigned int x, unsigned int y, uint32_t color);
void draw_char(unsigned int x, unsigned int y, unsigned char c, uint32_t color);
void print_string(const char *str, unsigned int x, unsigned int y, uint32_t color);
void print(char *str);

// gfx

struct window {
    int x;
    int y;
    int width;
    int height;
    int id;
    bool visible;
    int top_bar_h;
    int padding;
    int border;
    char *content;
    char *title;
    uint32_t color;
    uint32_t color_inactive;
    uint32_t border_color;
    uint32_t top_bar_color;
    uint32_t text_color;
    uint32_t text_color_inactive;
    uint32_t text_color_active;
};



extern struct window windows[];
extern int windows_count;  
extern const int windows_max;

// kernel

struct thread{
    int pid;
    int ppid;
    int priority;
    int state;
    int stack;
    int stack_size;
    int stack_base;
    int stack_limit;
    int lock_count;
    int process_pid;
};


struct process{
    int pid;
    int ppid;
    int priority;
    int state;
    int stack;
    int stack_size;
    int stack_base;
    int stack_limit;
    int lock_count;
    int thread_count;
    struct thread threads[];

};


void main(void);
extern int pid[];
extern int ready_queue[];
extern int ready_queue_count;
extern int current_task;
extern struct process processes[];
extern int process_count;
extern int thread_count;

// var_lock

extern bool *var_locks[];
extern int var_locks_count;


// var

#endif

