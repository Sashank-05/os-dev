#include <global.h>
#include <kernel.h>
#include <boot/limine.h>
#include <graphics/window.h>

extern struct window windows[];
extern int windows_count;
extern const int windows_max;
extern int cursor_x;
extern int cursor_y;
extern struct limine_framebuffer *framebuffer;

extern uint32_t windowbackup[10][1920 * 1080];

uint32_t windowbackup[10][1920 * 1080];

void save_window_backup(int window_index, int x, int y, int width, int height)
{
    uint32_t *fb_base = (uint32_t *)framebuffer->address;
    int fb_width = framebuffer->width;

    // Loop over the window's height and width
    for (int i = 0; i < height + windows[window_index].top_bar_h; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Ensure we are within framebuffer bounds
            if ((x + j) < fb_width && (y + i) < framebuffer->height)
            {
                // Copy pixel from framebuffer to windowbackup
                windowbackup[window_index][i * width + j] = fb_base[(y + i) * fb_width + (x + j)];
            }
        }
    }
}

void restore_window_backup(int window_index, int x, int y, int width, int height)
{
    uint32_t *fb_base = (uint32_t *)framebuffer->address;
    int fb_width = framebuffer->width;

    // Loop over the window's height and width
    for (int i = 0; i < height + windows[window_index].top_bar_h; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Ensure we are within framebuffer bounds
            if ((x + j) < fb_width && (y + i) < framebuffer->height)
            {
                // Restore pixel from windowbackup to framebuffer
                fb_base[(y + i) * fb_width + (x + j)] = windowbackup[window_index][i * width + j];
            }
        }
    }
}

void draw_filled_rect(int x, int y, int width, int height, uint32_t color)
{
    uint32_t *fb_base = (uint32_t *)framebuffer->address;
    int fb_width = framebuffer->width;
    int fb_height = framebuffer->height;

    if (x < 0)
    {
        width += x;
        x = 0;
    }
    if (y < 0)
    {
        height += y;
        y = 0;
    }
    if (x + width > fb_width)
    {
        width = fb_width - x;
    }
    if (y + height > fb_height)
    {
        height = fb_height - y;
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            fb_base[(y + i) * fb_width + (x + j)] = color;
        }
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
    uint32_t *fb_base = (uint32_t *)framebuffer->address;
    int fb_width = framebuffer->width;
    int fb_height = framebuffer->height;

    if (x < 0)
    {
        width += x;
        x = 0;
    }
    if (y < 0)
    {
        height += y;
        y = 0;
    }
    if (x + width > fb_width)
    {
        width = fb_width - x;
    }
    if (y + height > fb_height)
    {
        height = fb_height - y;
    }

    for (int j = 0; j < width; j++)
    {
        fb_base[y * fb_width + (x + j)] = color;
        fb_base[(y + height - 1) * fb_width + (x + j)] = color;
    }

    for (int i = 0; i < height; i++)
    {
        fb_base[(y + i) * fb_width + x] = color;
        fb_base[(y + i) * fb_width + (x + width - 1)] = color;
    }
}

void draw_line(int x1, int y1, int x2, int y2, uint32_t color)
{
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1)
    {
        draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

struct window createWindow(int x, int y, int width, int height, uint32_t color, char *title, char *content)
{
    struct window win;
    win.id = windows_count;
    win.x = x;
    win.y = y;
    win.width = width;
    win.height = height;
    win.color = color;
    win.title = title;
    win.top_bar_h = 20;
    win.content = content;
    win.exists = true;
    win.active = true;
    win.keep_on_top = false;
    win.content_type = TEXT;

    save_window_backup(windows_count, x, y, width, height);

    if (windows_count < windows_max)
    {
        windows[windows_count] = win;
        windows_count++;
    }
    else
    {
        return win;
    }

    return win;
}

struct window createGWindow(int x, int y, int width, int height, uint32_t color, char *title, uint64_t *content_buffer)
{
    struct window win;
    win.id = windows_count;
    win.x = x;
    win.y = y;
    win.width = width;
    win.height = height;
    win.color = color;
    win.title = title;
    win.top_bar_h = 20;
    win.content_buffer = content_buffer;
    win.exists = true;
    win.active = true;
    win.keep_on_top = false;
    win.content_type = GRAPHICS;

    save_window_backup(windows_count, x, y, width, height);

    if (windows_count < windows_max)
    {
        windows[windows_count] = win;
        windows_count++;
    }
    else
    {
        return win;
    }

    return win;
}

void draw_window(struct window *win)
{
    if (!win->exists)
    {
        return;
    }
    draw_filled_rect(win->x, win->y + win->top_bar_h, win->width, win->height, win->color);

    uint32_t top_bar_color = 0xFFAAAAAA;
    draw_filled_rect(win->x, win->y, win->width, win->top_bar_h, top_bar_color);

    uint32_t border_color = 0xFFAAAAAA;
    draw_rect(win->x, win->y, win->width, win->height + win->top_bar_h, border_color);

    if (win->title != NULL)
    {
        print_string(win->title, win->x + 5, win->y + (win->top_bar_h / 2) - 4, 0xFF000000);
    }

    int button_width = 15;
    int button_height = 15;

    int x_button_x = win->x + win->width - button_width - 5;
    int minimize_button_x = x_button_x - button_width - 5;
    int button_y = win->y + (win->top_bar_h - button_height) / 2;

    uint32_t button_color = 0xFF888888;
    draw_filled_rect(x_button_x, button_y, button_width, button_height, button_color);
    draw_rect(x_button_x, button_y, button_width, button_height, 0xFFFFFFFF);

    draw_line(x_button_x + 3, button_y + 3, x_button_x + button_width - 3, button_y + button_height - 3, 0xFFFFFFFF);
    draw_line(x_button_x + button_width - 3, button_y + 3, x_button_x + 3, button_y + button_height - 3, 0xFFFFFFFF);

    draw_filled_rect(minimize_button_x, button_y, button_width, button_height, button_color);
    draw_rect(minimize_button_x, button_y, button_width, button_height, 0xFFFFFFFF);

    draw_line(minimize_button_x + 3, button_y + button_height / 2, minimize_button_x + button_width - 3, button_y + button_height / 2, 0xFFFFFFFF);

    if (win->content_type == TEXT)
    {
        print_string(win->content, win->x + 10, win->y + win->top_bar_h + 10, 0x0000000);

        if (win->content != NULL)
        {
            print_string(win->content, win->x + 10, win->y + win->top_bar_h + 10, 0x0000000);
        }
        else
        {
            print("No content");
        }
    }
    elif (win->content_type == GRAPHICS)
    {
        uint32_t *fb_base = (uint32_t *)framebuffer->address;
        int fb_width = framebuffer->width;
        int fb_height = framebuffer->height;

        for (int i = win->top_bar_h; i < win->height + win->top_bar_h; i++)
        {
            for (int j = 0; j < win->width; j++)
            {
                if ((win->x + j) < fb_width && (win->y + i) < fb_height)
                {
                    fb_base[(win->y + i) * fb_width + (win->x + j)] = win->content_buffer[(i - win->top_bar_h) * win->width + j];
                }
            }
        }
    }
}
void draw_rounded_rect(int x, int y, int width, int height, int radius, uint32_t color)
{
    // Draw the sides (without corners)
    draw_line(x + radius, y, x + width - radius, y, color);                   // Top side
    draw_line(x + radius, y + height, x + width - radius, y + height, color); // Bottom side
    draw_line(x, y + radius, x, y + height - radius, color);                  // Left side
    draw_line(x + width, y + radius, x + width, y + height - radius, color);  // Right side

    // Draw the rounded corners using quarter circles
    draw_circle_quadrant(x + radius, y + radius, radius, 1, color);                  // Top-left corner
    draw_circle_quadrant(x + width - radius, y + radius, radius, 2, color);          // Top-right corner
    draw_circle_quadrant(x + radius, y + height - radius, radius, 3, color);         // Bottom-left corner
    draw_circle_quadrant(x + width - radius, y + height - radius, radius, 4, color); // Bottom-right corner
}

// Function to draw a quarter circle based on the quadrant
void draw_circle_quadrant(int xc, int yc, int radius, int quadrant, uint32_t color)
{
    int x = 0, y = radius;
    int d = 1 - radius;

    while (y >= x)
    {
        // Top-left quadrant (1)
        if (quadrant == 1)
        {
            draw_pixel(xc - x, yc - y, color);
            draw_pixel(xc - y, yc - x, color);
        }
        // Top-right quadrant (2)
        else if (quadrant == 2)
        {
            draw_pixel(xc + x, yc - y, color);
            draw_pixel(xc + y, yc - x, color);
        }
        // Bottom-left quadrant (3)
        else if (quadrant == 3)
        {
            draw_pixel(xc - x, yc + y, color);
            draw_pixel(xc - y, yc + x, color);
        }
        // Bottom-right quadrant (4)
        else if (quadrant == 4)
        {
            draw_pixel(xc + x, yc + y, color);
            draw_pixel(xc + y, yc + x, color);
        }

        if (d < 0)
        {
            d += 2 * x + 3;
        }
        else
        {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void draw_circle(int xc, int yc, int r, uint32_t color)
{
    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (y >= x)
    {
        // Draw symmetrical points in all octants
        draw_pixel(xc + x, yc + y, color);
        draw_pixel(xc - x, yc + y, color);
        draw_pixel(xc + x, yc - y, color);
        draw_pixel(xc - x, yc - y, color);
        draw_pixel(xc + y, yc + x, color);
        draw_pixel(xc - y, yc + x, color);
        draw_pixel(xc + y, yc - x, color);
        draw_pixel(xc - y, yc - x, color);

        x++;

        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}

void draw_filled_circle(int xc, int yc, int r, uint32_t color)
{
    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (y >= x)
    {
        // Draw horizontal lines between points to fill the circle
        draw_line(xc - x, yc + y, xc + x, yc + y, color); // Top horizontal part of the circle
        draw_line(xc - x, yc - y, xc + x, yc - y, color); // Bottom horizontal part of the circle
        draw_line(xc - y, yc + x, xc + y, yc + x, color); // Left vertical part of the circle
        draw_line(xc - y, yc - x, xc + y, yc - x, color); // Right vertical part of the circle

        x++;

        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}

void draw_filled_rounded_rect(int x, int y, int width, int height, int radius, uint32_t color)
{
    // Draw the middle part (without rounded corners)
    draw_filled_rect(x + radius, y, width - 2 * radius, height, color); // Middle part
    draw_filled_rect(x, y + radius, width, height - 2 * radius, color); // Top and bottom part

    // Draw the corners with circles/arcs
    draw_filled_circle(x + radius, y + radius, radius, color);                  // Top-left corner
    draw_filled_circle(x + width - radius, y + radius, radius, color);          // Top-right corner
    draw_filled_circle(x + radius, y + height - radius, radius, color);         // Bottom-left corner
    draw_filled_circle(x + width - radius, y + height - radius, radius, color); // Bottom-right corner
}

void draw_rounded_window(struct window *win)
{
    if (!win->exists)
    {
        return;
    }

    int corner_radius = 10;

    draw_filled_rounded_rect(win->x, win->y + win->top_bar_h, win->width, win->height, corner_radius, win->color);

    uint32_t top_bar_color = 0xFFAAAAAA;
    draw_filled_rounded_rect(win->x, win->y, win->width, win->top_bar_h, corner_radius, top_bar_color);

    uint32_t border_color = 0xFFAAAAAA;
    draw_rounded_rect(win->x, win->y, win->width, win->height + win->top_bar_h, corner_radius, border_color);

    if (win->title != NULL)
    {
        print_string(win->title, win->x + 5, win->y + (win->top_bar_h / 2) - 4, 0xFF000000);
    }

    int button_width = 15;
    int button_height = 15;

    int x_button_x = win->x + win->width - button_width - 5;
    int minimize_button_x = x_button_x - button_width - 5;
    int button_y = win->y + (win->top_bar_h - button_height) / 2;

    uint32_t button_color = 0xFF888888;
    draw_filled_circle(x_button_x + button_width / 2, button_y + button_height / 2, button_width / 2, button_color);
    draw_circle(x_button_x + button_width / 2, button_y + button_height / 2, button_width / 2, 0xFFFFFFFF);

    draw_line(x_button_x + 3, button_y + 3, x_button_x + button_width - 3, button_y + button_height - 3, 0xFFFFFFFF);
    draw_line(x_button_x + button_width - 3, button_y + 3, x_button_x + 3, button_y + button_height - 3, 0xFFFFFFFF);

    draw_filled_circle(minimize_button_x + button_width / 2, button_y + button_height / 2, button_width / 2, button_color);
    draw_circle(minimize_button_x + button_width / 2, button_y + button_height / 2, button_width / 2, 0xFFFFFFFF);

    draw_line(minimize_button_x + 3, button_y + button_height / 2, minimize_button_x + button_width - 3, button_y + button_height / 2, 0xFFFFFFFF);

    if (win->content_type == TEXT)
    {
        print_string(win->content, win->x + 10, win->y + win->top_bar_h + 10, 0x0000000);

        if (win->content != NULL)
        {
            print_string(win->content, win->x + 10, win->y + win->top_bar_h + 10, 0x0000000);
        }
        else
        {
            print("No content");
        }
    }
    elif (win->content_type == GRAPHICS)
    {
        uint32_t *fb_base = (uint32_t *)framebuffer->address;
        int fb_width = framebuffer->width;
        int fb_height = framebuffer->height;

        // copy the content buffer to the framebuffer
        for (int i = win->top_bar_h; i < win->height + win->top_bar_h; i++){
            for (int j = 0; j < win->width; j++){
                fb_base[(win->y + i) * fb_height + (win->x + j)] = win->content_buffer[i * win->width + j];

            }
        }
        
    }
}

void move_window(struct window *win, int x, int y)
{
    if (!win->exists)
    {
        return;
    }
    restore_window_backup(win->id, win->x, win->y, win->width, win->height);
    win->x = x;
    win->y = y;
    save_window_backup(win->id, win->x, win->y, win->width, win->height);
}

void destroy_window(struct window *win)
{
    if (!win->exists)
    {
        return;
    }
    restore_window_backup(win->id, win->x, win->y, win->width, win->height);
    save_window_backup(win->id, win->x, win->y, win->width, win->height);
    win->exists = false;
    // move all the windows in the array to the left
    for (int i = 0; i < windows_count; i++)
    {
        if (windows[i].id == win->id)
        {
            for (int j = i; j < windows_count - 1; j++)
            {
                windows[j] = windows[j + 1];
            }
        }
    }
    // windows_count--;
}

void keep_on_top(struct window *win)
{
    if (!win->exists)
    {
        return;
    }
    win->keep_on_top = true;
    // move window to the end of the array
    for (int i = 0; i < windows_count; i++)
    {
        if (windows[i].id == win->id)
        {
            for (int j = i; j < windows_count - 1; j++)
            {
                struct window temp = windows[j];
                windows[j] = windows[j + 1];
                windows[j + 1] = temp;
            }
        }
    }
}

void make_active(struct window *win)
{
    if (!win->exists)
    {
        return;
    }
    win->active = true;
    for (int i = 0; i < windows_count; i++)
    {
        if (windows[i].id == win->id)
        {
            for (int j = i; j < windows_count - 1; j++)
            {
                struct window temp = windows[j];
                windows[j] = windows[j + 1];
                windows[j + 1] = temp;
            }
        }
    }
}