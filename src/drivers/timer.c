#include <timer.h>
#include <io.h>
#include <util.h>
#include <isr.h>
#include <boot_print.h>

#define MAX_TIMERS 255

typedef struct
{
    uint32_t frequency;
    uint32_t remaining_ticks;
    void (*callback)(void);
} software_timer_t;

uint32_t tick = 0;
software_timer_t timers[MAX_TIMERS];

static void timer_callback(registers_t *regs)
{
    tick++;

    for (int i = 0; i < MAX_TIMERS; i++)
    {
        if (timers[i].callback != NULL)
        {
            if (timers[i].remaining_ticks == 0)
            {
                timers[i].callback();
                timers[i].remaining_ticks = timers[i].frequency;
            }
            else
            {
                timers[i].remaining_ticks--;
            }
        }
    }
}

void init_hardware_timer(uint32_t freq)
{
    register_interrupt_handler(IRQ0, timer_callback);

    uint32_t divisor = 1193180 / freq;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x43, 0x36);
    outb(0x40, low);
    outb(0x40, high);
}

int add_timer(uint32_t freq_in_ticks, void (*callback)(void))
{
    for (int i = 0; i < MAX_TIMERS; i++)
    {
        if (timers[i].callback == NULL)
        {
            timers[i].frequency = freq_in_ticks;
            timers[i].remaining_ticks = freq_in_ticks;
            timers[i].callback = callback;
            return 0;
        }
    }
    return -1;
}

int remove_timer(void (*callback)(void))
{
    for (int i = 0; i < MAX_TIMERS; i++)
    {
        if (timers[i].callback == callback)
        {
            timers[i].callback = NULL; // Remove the timer by setting the callback to NULL
            return 0;                  // Timer successfully removed
        }
    }
    return -1; // Timer not found
}
