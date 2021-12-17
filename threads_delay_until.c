#include "demo_bsp.h"
#include <FXRTOS.h>
#include <stdio.h>
#include <string.h>

uint32_t time_0;
uint32_t time_1;

extern void led_on(void);
extern void led_off(void);

// This task starts first.
// After tick count is bigger than value in time_0, led turns on.
// New value of time_0 is previous value + 2000, so thread waits 2000 ticks
// before next turning led on.
void
task_0(void* args)
{
    for (uint8_t i = 0; i < 10; i++)
    {
        fx_thread_delay_until(&time_0, 2000);
        led_on();
    }
    fx_thread_exit();
}

// Here is the same idea like in task_0 but led turn off.
// Since initial value of time_1 is time_0 + 1000, this gives blinking with 
// 2000 ticks period.
void
task_1(void* args)
{
    for (uint8_t i = 0; i < 10; i++)
    {
        fx_thread_delay_until(&time_1, 2000);
        led_off();
    }
    fx_thread_exit();
}

void
task_main(void* args)
{
    // Creating threads and their stacks.
    static fx_thread_t thread_0;
    static fx_thread_t thread_1;
    static uint32_t stack_0[0x200 / sizeof(uint32_t)];
    static uint32_t stack_1[0x200 / sizeof(uint32_t)];
    
    // Initializing threads in suspended mode.
    fx_thread_init(&thread_0, task_0, NULL, 10, stack_0, sizeof(stack_0), true);
    fx_thread_init(&thread_1, task_1, NULL, 10, stack_1, sizeof(stack_1), true);

    // Setting initial activation time for threads.
    // This value is needed for delay_until function. Threads aren't really.
    // activated first time at this time.
    time_0 = fx_timer_get_tick_count() + 100;
    time_1 = time_0 + 1000;

    // Starting threads.
    fx_thread_resume(&thread_0);
    fx_thread_resume(&thread_1);

    fx_thread_exit();
}

void
fx_app_init(void)
{
    // Creating and initializing main thread
    static fx_thread_t thread_main;
    static uint32_t stack_main[0x400 / sizeof(uint32_t)];
    fx_thread_init(&thread_main, task_main, NULL, 5, stack_main,
                   sizeof(stack_main), false);
}

void
fx_intr_handler(void)
{
    ;
}

int
main(void)
{
    demo_bsp_init();
    //
    // Kernel start. This function must be called with interrupts disabled.
    //
    fx_kernel_entry();
}
