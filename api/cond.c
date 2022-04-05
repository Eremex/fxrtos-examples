#include <stdio.h>
#include <FXRTOS.h>
#include "demo_bsp.h"

fx_cond_t cond_0;
fx_cond_t cond_1;
fx_cond_t cond_2;

fx_mutex_t mutex;

fx_thread_t thread_0;
fx_thread_t thread_1;
fx_thread_t thread_2;
fx_thread_t thread_3;

bool cond_0_var = 0;
bool cond_1_var = 0;

//
// This task is used by 2 of 4 threads. Here each thread stops on 2 condvars.
// Task_main opens all this condvars. While loop around fx_cond_wait allows
// pass through without starting to wait, if condition have been already 
// completed.
//
void
task_0(void* args)
{
    fx_mutex_acquire(&mutex, NULL);
    printf("Thread_%c started with task_0\n\r", *(char*)args);
    while (!cond_0_var)
    {
        printf("Cond_0 not done, so thread_%c goes to wait\n\r", *(char*)args);
        fx_cond_wait(&cond_0, &mutex, NULL);
    }

    printf("Thread_%c passed through cond_0\n\r", *(char*)args);

    fx_cond_wait(&cond_2, &mutex, NULL);
    printf("Thread_%c passed through cond_2\n\r", *(char*)args);
    fx_mutex_release(&mutex);

    fx_thread_exit();
}

//
// Here is the same idea as in task_0 but another condvar is used.
//
void
task_1(void* args)
{
    fx_mutex_acquire(&mutex, NULL);
    printf("Thread_%c started with task_1\n\r", *(char*)args);
    while (!cond_1_var)
    {
        printf("Cond_1 not done, so thread_%c goes to wait\n\r", *(char*)args);
        fx_cond_wait(&cond_1, &mutex, NULL);
    }

    printf("Thread_%c passed through cond_1\n\r", *(char*)args);

    fx_cond_wait(&cond_2, &mutex, NULL);
    printf("Thread_%c passed through cond_2\n\r", *(char*)args);
    fx_mutex_release(&mutex);

    fx_thread_exit();
}

//
// There is thread and condvar control in task_main.
//
void
task_main(void* args)
{
    printf("Task_main started\n\r");
    printf("Task_main starts 3 other threads\n\r");
    fx_thread_resume(&thread_0);
    fx_thread_resume(&thread_2);
    fx_thread_resume(&thread_3);
    fx_thread_sleep(1000);

    printf("\nTask_main completes cond_0 and lets one waiter to pass it\n\r");
    cond_0_var = 1;
    fx_cond_signal(&cond_0);
    fx_thread_sleep(1000);

    printf("\nTask_main starts last thread with already completed cond_0\n\r");
    fx_thread_resume(&thread_1);
    fx_thread_sleep(1000);

    printf("\nTask_main completes cond_1 and lets all waiters to pass it\n\r");
    cond_1_var = 1;
    fx_cond_broadcast(&cond_1);
    fx_thread_sleep(1000);

    printf("\nTask_main deinits cond_2 and lets all waiters to pass it\n\r");
    fx_cond_deinit(&cond_2);
    fx_thread_exit();
}

void
fx_app_init(void)
{
    //
    // Creating main thread and all stacks.
    //
    static fx_thread_t thread_main;
    static uint32_t stack_0[0x200 / sizeof(uint32_t)];
    static uint32_t stack_1[0x200 / sizeof(uint32_t)];
    static uint32_t stack_2[0x200 / sizeof(uint32_t)];
    static uint32_t stack_3[0x200 / sizeof(uint32_t)];
    static uint32_t stack_main[0x200 / sizeof(uint32_t)];
    //
    // Initializing threads, conds and mutex.
    //
    fx_cond_init(&cond_0, FX_SYNC_POLICY_DEFAULT);
    fx_cond_init(&cond_1, FX_SYNC_POLICY_DEFAULT);
    fx_cond_init(&cond_2, FX_SYNC_POLICY_DEFAULT);
    fx_mutex_init(&mutex, FX_MUTEX_CEILING_DISABLED, FX_SYNC_POLICY_DEFAULT);
    fx_thread_init(&thread_0, task_0, "0", 10, stack_0, sizeof(stack_0), 1);
    fx_thread_init(&thread_1, task_0, "1", 10, stack_1, sizeof(stack_1), 1);
    fx_thread_init(&thread_2, task_1, "2", 10, stack_2, sizeof(stack_2), 1);
    fx_thread_init(&thread_3, task_1, "3", 10, stack_3, sizeof(stack_3), 1);
    fx_thread_init(&thread_main, task_main, NULL, 10, stack_main,
                   sizeof(stack_main), 0);
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
