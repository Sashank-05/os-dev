#include <global.h>
#include <kernel.h>
#include <boot/limine.h>

struct window createWindow(int x, int y, int width, int height, uint32_t color, char *title, char *content);
void draw_window(struct window *win);


