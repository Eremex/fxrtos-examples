#include "demo_bsp.h"
#include <FXRTOS.h>
#include <stdio.h>
#include <string.h>

// Creating semaphore.
fx_sem_t sem_block;

extern void led_on(void);
extern void led_off(void);

// This task starts first. As initial value of sem_block is 0, timedwait causes
// blocking of thread.
void
task_0(void *args)
{
    int res;
    printf("Task_0 started\n\r");
    res = fx_sem_timedwait(&sem_block, FX_THREAD_INFINITE_TIMEOUT);
    printf("Task_0 continued\n\r");
    // As sem_block was released in specific way, timedwait res isn't FX_SEM_OK.
    if (res == FX_THREAD_WAIT_DELETED)
    {
        printf("Semaphore was deinited\n\r");
    }
    fx_thread_exit();
}

// When thread_0 blocks this task starts. Setting timeout value prevents 
// infinite bocking. After 5 tryes of wait sem_block is released with deinit.
// This frees all threads blocked with deinited semaphore. Before using deinited
// semaphore it must be reinited.
void
task_1(void *args)
{
    for (unsigned i = 0; i < 5; i++)
    {
        printf("Trying to wait sem_block\n\r");
        fx_sem_timedwait(&sem_block, 1000);
    }
    printf("Releasing sem_block\n\r");
    fx_sem_deinit(&sem_block);
    fx_thread_exit();
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
    fx_sem_init(&sem_block, 0, 1, FX_SYNC_POLICY_FIFO);
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
