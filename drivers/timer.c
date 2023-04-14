#include "irq.h"
#include "timer.h"
#include "screen.h"
#include "low_level.h"
#include "mouse.h"

void timer_phase(int hz)
{
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

/* This will keep track of how many ticks that the system
*  has been running for */
volatile unsigned int timer_ticks = 0;

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. */
void timer_handler(struct regs *r)
{
    /* Increment our 'tick count' */
    timer_ticks++;
    if (is_screen_initialized())
        set_timer_ticks(timer_ticks);
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
    /* Installs 'timer_handler' to IRQ0 */
    timer_phase(PHASE); // Set timer to call irq0 PHASE times a second.
    irq_install_handler(0, timer_handler);
}

void timer_wait(int seconds)
{

    timer_ticks = 0;

    while (timer_ticks < seconds*PHASE);

}

void wait_milliseconds(int milliseconds)
{

    timer_ticks = 0;

    while (timer_ticks*10 < milliseconds);

}