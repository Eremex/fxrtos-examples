#include <stdio.h>
#include <FXRTOS.h>
#include "demo_bsp.h"

fx_event_t event;
fx_sem_t semaphore;

//
// This task waits for event activations.
//
void
task_0(void* args)
{
    printf("Thread_0 started\n\r");

    printf("Thread_0 going to wait for event\n\r");
    fx_thread_wait_event(&event, NULL);
    printf("Thread_0 went through event\n\r");

    bool event_state;
    fx_event_get_state(&event, &event_state);
    if (event_state)
    {
        printf("Thread_0 resets event\n\r");
        fx_event_reset(&event);
    }
    //
    // Using cancelling event.
    //
    printf("Thread_0 going to wait for semaphore\n\r");
    fx_sem_wait(&semaphore, &event);
    printf("Thread_0 went through semaphore\n\r");

    fx_thread_exit();
}

//
// This task activates event.
//
void
task_1(void* args)
{
    printf("Thread_1 started\n\r");

    printf("Thread_1 activates event 1st time\n\r");
    fx_event_set(&event);
    
    fx_thread_yield();

    printf("Thread_1 activates event 2nd time\n\r");
    fx_event_set(&event);

    fx_thread_exit();
}

void
fx_app_init(void)
{
    //
    // Creating threads and their stacks.
    //
    static fx_thread_t thread_0;
    static fx_thread_t thread_1;
    static uint32_t stack_0[0x200 / sizeof(uint32_t)];
    static uint32_t stack_1[0x200 / sizeof(uint32_t)];
    //
    // Initializing threads, semaphore and event.
    //
    fx_event_init(&event, 0);
    fx_sem_init(&semaphore, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_thread_init(&thread_0, task_0, NULL, 10, stack_0, sizeof(stack_0), 0);
    fx_thread_init(&thread_1, task_1, NULL, 10, stack_1, sizeof(stack_1), 0);
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
