#include <stdio.h>
#include <FXRTOS.h>
#include "demo_bsp.h"

void
task(void *args)
{
    unsigned selfPrio;
    //
    // Pointer to thread can be obtained from it's task with thread_self.
    // Some thread params can be obtained with thread_get_params.
    //
    fx_thread_get_params(fx_thread_self(), FX_THREAD_PARAM_PRIO, &selfPrio);
    printf("Hello from task with prio = %u\n\r", selfPrio);
    fx_thread_exit();
}

void
task_main(void *args)
{
    //
    // Creating thread and it's stack.
    //
    static fx_thread_t thread;
    static uint32_t stack[0x200 / sizeof(uint32_t)];

    //    
    // Initializing thread with task and priority level 10.
    //
    fx_thread_init(&thread, task, NULL, 10, stack, sizeof(stack), false);
    //
    // Waiting for thread's task ending.
    //
    fx_thread_join(&thread);
    //
    // Thread structure and it's stack can be reused but deinit must be called
    // before.
    //
    fx_thread_deinit(&thread);
    //
    // Reinitializing thread with other priority level.
    //
    fx_thread_init(&thread, task, NULL, 9, stack, sizeof(stack), false);
    fx_thread_join(&thread);

    fx_thread_deinit(&thread);
    fx_thread_init(&thread, task, NULL, 10, stack, sizeof(stack), false);
    //
    // Priority level can be changed with set_params after initializing thread.
    //
    fx_thread_set_params(&thread, FX_THREAD_PARAM_PRIO, 7);
    fx_thread_join(&thread);

    fx_thread_exit();
}

void
fx_app_init(void)
{
    static fx_thread_t thread_main;
    static uint32_t stack_main[0x400 / sizeof(uint32_t)];
    fx_thread_init(&thread_main, task_main, NULL, 6, stack_main, 
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
