#ifndef GLOBAL

#define GLOBAL

#define elif else if

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// boot
extern struct limine_framebuffer *framebuffer;



// gfx

struct window {
    int x;
    int y;
    int w;
    int h;
    int id;
    bool visible;
    int top_bar_h;
    int padding;
    int border;
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
extern int windows_max;

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

