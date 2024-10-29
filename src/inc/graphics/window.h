#include <global.h>
#include <kernel.h>
#include <boot/limine.h>

extern struct window windows[];
extern int windows_count;
extern const int windows_max;
extern int cursor_x;
extern int cursor_y;
extern struct limine_framebuffer* framebuffer;
extern uint32_t windowbackup[10][1920*1080];


void save_window_backup(int window_index, int x, int y, int width, int height);
void restore_window_backup(int window_index, int x, int y, int width, int height);
struct window createWindow(int x, int y, int width, int height, uint32_t color, char *title, char *content);
struct window createGWindow(int x, int y, int width, int height, uint32_t color, char *title, uint64_t *content_buffer);
void draw_window(struct window *win);
void draw_filled_rect(int x, int y, int width, int height, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);

void draw_filled_circle(int xc, int yc, int r, uint32_t color);
void draw_circle(int xc, int yc, int r, uint32_t color);
void draw_circle_quadrant(int xc, int yc, int r, int quadrant, uint32_t color);
void draw_rounded_rect(int x, int y, int width, int height, int radius, uint32_t color);
void draw_rounded_window(struct window *win);

void move_window(struct window *win, int x, int y);
void destroy_window(struct window *win);
void keep_on_top(struct window *win);
void make_active(struct window *win);


