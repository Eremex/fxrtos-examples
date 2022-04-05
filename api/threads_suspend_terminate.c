#include <stdio.h>
#include <FXRTOS.h>
#include "demo_bsp.h"

//
// This task starts after thread_1 is suspended. Thread_0 will be killed by
// thread_1 after it's resuming. As task_main sleeps 5000 ticks before resuming
// thread_1, led will be turned on for ~5000 ticks.
//
void
task_0(void* args)
{
    printf("Task_0 started\n\r");
    for (;;)
    {
        led_on();
        fx_thread_yield();
    }
}

//
// This task starts first.
// After first printf thread goes into suspended mode. It will be resumed after
// 5000 ticks from task_main.
// After resuming fx_thread_terminate is used to kill thread_0, passed by 
// pointer in thread initializing function. After killing thread_0 led is
// turned off.
//

void
task_1(void* target)
{
    printf("Task_1 started\n\r");

    //
    // Going into suspended mode.
    //
    fx_thread_suspend();

    printf("Task_1 restarted\n\r");

    //
    // Killing thread_0.
    //
    fx_thread_terminate(target);
    led_off();

    fx_thread_exit();
}

void
task_main(void* args)
{
    //
    // Creating threads and their stacks.
    //
    static fx_thread_t thread_0;
    static fx_thread_t thread_1;
    static uint32_t stack_0[0x200 / sizeof(uint32_t)];
    static uint32_t stack_1[0x200 / sizeof(uint32_t)];
    
    //
    // Initializing threads in suspended mode.
    // Pointer to thread_0 is passed to thread_1 as argument.
    //
    fx_thread_init(&thread_0, task_0, NULL, 10, stack_0, sizeof(stack_0), true);
    fx_thread_init(&thread_1, task_1, &thread_0, 10, stack_1, sizeof(stack_1), true);

    //
    // Starting threads in other order than they were initialized.
    //
    fx_thread_resume(&thread_1);
    fx_thread_resume(&thread_0);

    //
    // Waiting for 5000 ticks and then resuming thread_1.
    //
    fx_thread_sleep(5000);
    fx_thread_resume(&thread_1);

    fx_thread_exit();
}

void
fx_app_init(void)
{
    static fx_thread_t thread_main;
    static uint32_t stack_main[0x400 / sizeof(uint32_t)];
    fx_thread_init(&thread_main, task_main, NULL, 9, stack_main, 
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
    //
    // Hardware modules initialization
    //
    core_init();
    led_init();
    console_init();
    //timer_init();
    //
    // Kernel start. This function must be called with interrupts disabled.
    //
    fx_kernel_entry();
}
