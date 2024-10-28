#include <global.h>
#include <kernel.h>
#include <graphics/window.h>
#include <boot/limine.h>

extern const int windows_max;
extern struct window windows[];
extern int windows_count;

const int windows_max = 10;
struct window windows[200];
int windows_count = 0;

extern int cursor_x;
extern int cursor_y;

void main(void)
{
    createWindow(45, 45, 200, 200, 0xFFFFFF, "Test 1", "This is a test window");
    createWindow(400, 200, 600, 400, 0xFFFFFF, "Test 2", "This is another test window");
    while (1)
    {
        for (int i = 0; i < windows_count; i++)
        {
            draw_window(&windows[i]);
        }

        // wait for 100ms
        for (int i = 0; i < 100000000; i++)
        {
            asm("nop");
        }
    }
    
}
