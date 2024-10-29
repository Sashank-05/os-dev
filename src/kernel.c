#include <global.h>
#include <kernel.h>
#include <graphics/window.h>
// #include <boot/animate.h>
#include <boot/limine.h>
#include <graphics/loading.h>

extern const int windows_max;
extern struct window windows[];
extern int windows_count;

extern struct process processes[];

const int windows_max = 10;
struct window windows[200];
int windows_count = 0;

extern int cursor_x;
extern int cursor_y;

int frame = 0;

void main(void)
{


}
