#include <cpu/timer.h>
#include <drivers/screen.h>
#include <drivers/io.h>
#include <util.h>
#include <cpu/isr.h>

uint32_t tick = 0;
extern int curr_line;

static void timer_callback(registers_t *regs) {
    tick++;
    boot_print("Tick: ");

    char tick_ascii[256];
    itoa(tick, tick_ascii);
    boot_print(tick_ascii);
    boot_print("\n");
}

void init_timer(uint32_t freq) {
    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)( (divisor >> 8) & 0xFF);
    /* Send the command */
    port_byte_out(0x43, 0x36); /* Command port */
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}