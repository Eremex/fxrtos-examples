#include <stdio.h>
#include <FXRTOS.h>
#include "demo_bsp.h"

fx_mutex_t mutex;

//
// This task starts first. It blocks mutex and goes into suspended mode.
// Later it will be resumed by thread_1.
//
void
task_0(void* args)
{
    fx_mutex_acquire(&mutex, NULL);
    printf("Mutex acquired by thread_0\n\r");
    printf("Thread_0 goes into suspended mode\n\r");
    fx_thread_suspend();
    printf("Thread_0 resumed\n\r");
    fx_mutex_release(&mutex);
    printf("Mutex released by thread_0\n\r");
    fx_thread_exit();
}

//
// This task tryes to acquire mutex. As it is already acquired by thread_0,
// timedacqure returns FX_THREAD_WAIT_TIMEOUT. To release mutex it's owner
// is obtained with get_owner function.
//
void
task_1()
{
    fx_thread_t* mutex_owner;
    printf("Trying to acquire mutex from thread_1\n\r");
    if (fx_mutex_timedacquire(&mutex, 2000) != FX_THREAD_WAIT_TIMEOUT)
    {
        printf("Something gone wrong...\n\r");
        fx_thread_exit();
    }

    printf("Mutex is blocked for at least 2000 ticks\n\r");
    mutex_owner = fx_mutex_get_owner(&mutex);
    fx_thread_resume(mutex_owner);
    printf("Mutex owner resumed by thread_1\n\r");
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
    // Initializing threads and mutex.
    //
    fx_mutex_init(&mutex, FX_MUTEX_CEILING_DISABLED, FX_SYNC_POLICY_DEFAULT);
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
