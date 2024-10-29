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
    createWindow(100, 100, 300, 300, 0xFFFFFF, "Test 3", "This is a third test window");
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
        // simlate moving the window
        if (windows[1].x < framebuffer->width - windows[1].width)
        {
            move_window(&windows[1], windows[1].x + 5, windows[1].y + 1);
        }
        else
        {
            for (int i = 0; i < 1000000000; i++)
            {
                asm("nop");
            }
            destroy_window(&windows[2]);
            //make_active(&windows[0]);

            print("Destroyed window 2");
        }
    }
}
