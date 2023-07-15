#include "irq.h"
#include "timer.h"
#include "screen.h"
#include "low_level.h"
#include "mouse.h"
#include "strings.h"

void timer_phase(int hz)
{
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, (divisor >> 8)&0xFF);     /* Set high byte of divisor */
}

/* This will keep track of how many ticks that the system
*  has been running for */
volatile unsigned int timer_ticks = 0;
unsigned int current_sec = 0;
unsigned int current_min = 0;
unsigned int current_hour = 0;
unsigned int second_ticks = 0;

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. */
void timer_handler()
{
    /* Increment our 'tick count' */
    timer_ticks++;
    second_ticks++;
    update_time();
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
    current_sec = 0;
    current_min = 0;
    current_hour = 0;
    second_ticks = 0;
    timer_ticks = 0;
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

    while (timer_ticks*(1000/PHASE) < milliseconds);

}

void set_time(unsigned int hour, unsigned int minute)
{

    current_hour = hour % 24;
    current_min = minute % 60;
    current_sec = 0;
    second_ticks = 0;

}

void update_time()
{
    if(second_ticks % PHASE == 0)
    {
        second_ticks = 0;
        current_sec += 1;
        if(current_sec == 60) // phase is how many times a second * 60 seconds.
        {   
            current_sec = 0;
            current_min += 1;
            if (current_min == 60)
            {

                current_min = 0;
                current_hour = (current_hour+1) % 24;

            }
        }

        char hour_str[2 + 1];
        int_to_str_padding(current_hour,hour_str,10,2);

        char min_str[2 + 1];
        int_to_str_padding(current_min,min_str,10,2);

        char second_str[2 + 1];
        int_to_str_padding(current_sec,second_str,10,2);

        int cursor_coords = get_cursor();
        set_cursor_coords(TIME_OFF,0);
        screen_printf("%s:%s:%s",hour_str,min_str,second_str);
        set_cursor(cursor_coords);
    }
}