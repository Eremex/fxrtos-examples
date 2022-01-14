#include "demo_bsp.h"
#include <FXRTOS.h>
#include <stdio.h>
#include <string.h>

// Creating semaphores.
fx_sem_t sem_on;
fx_sem_t sem_off;
fx_sem_t sem_counter;

extern void led_on(void);
extern void led_off(void);

// Function for loading cpu for some time.
void
do_work(void (*f)(void))
{
    for(unsigned i = 0; i < 50000; i++)
    {
        f();
    }
}

// This task turns led on for some time. Two semaphores are used for 
// synchronization with second thread.
void
task_0(void* args)
{
    for (;;)
    {
        fx_sem_timedwait(&sem_off, FX_THREAD_INFINITE_TIMEOUT);
        printf("Thread_0 turned led on\n\r");
        do_work(led_on);
        fx_sem_post(&sem_on);
    }
}

// This task turns led off for some time. Also it uses sem_counter for counting
// blinks. After 100 blinks thread will be blocked.
void
task_1()
{
    unsigned counter_value;
    for (;;)
    {
        fx_sem_wait(&sem_on, NULL);
        printf("Thread_1 turned led off\n\r");
        do_work(led_off);
        fx_sem_get_value(&sem_counter, &counter_value);
        printf("LED blinked %u times\n\r", 100 - counter_value);
        fx_sem_wait(&sem_counter, NULL);
        fx_sem_post(&sem_off);
    }
}

void
fx_app_init(void)
{
    // Creating threads and their stacks.
    static fx_thread_t thread_0;
    static fx_thread_t thread_1;
    static uint32_t stack_0[0x400 / sizeof(uint32_t)];
    static uint32_t stack_1[0x400 / sizeof(uint32_t)];

    // Initializing threads and semaphores.
    fx_sem_init(&sem_on, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_sem_init(&sem_off, 1, 1, FX_SYNC_POLICY_FIFO);
    fx_sem_init(&sem_counter, 99, 99, FX_SYNC_POLICY_FIFO);
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
    demo_bsp_init();
    //
    // Kernel start. This function must be called with interrupts disabled.
    //
    fx_kernel_entry();
}
