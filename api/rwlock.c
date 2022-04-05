#include <stdio.h>
#include <stdlib.h>
#include <FXRTOS.h>
#include "demo_bsp.h"

fx_rwlock_t rwlock;
char buf[64];

//
// This task reads buf content and prints it to output.
// Using rwlock allows several readers work at the same time.
//
void
buf_read(void* args)
{
    for (;;)
    {
        fx_rwlock_rd_lock(&rwlock, NULL);
        printf("Thread_%c read from buf: %s\n\r", *(char*)args, buf);
        fx_rwlock_unlock(&rwlock);
        fx_thread_sleep(1000 + rand() % 5000);
    }
}

//
// This task writes text to buf. Using rwlock disallows other writers or readers
// to access buf while one of writers is working.
//
void
buf_write(void* args)
{
    unsigned curr_time;
    for (;;)
    {
        fx_rwlock_wr_lock(&rwlock, NULL);
        curr_time = fx_timer_get_tick_count();
        sprintf(buf, "Thread_%c wrote at time %u", *(char*)args, curr_time);
        fx_rwlock_unlock(&rwlock);
        fx_thread_sleep(2000 + rand() % 5000);
    }
}

void
fx_app_init(void)
{
    //
    // Creating threads and their stacks.
    //
    static fx_thread_t thread_0;
    static fx_thread_t thread_1;
    static fx_thread_t thread_2;
    static fx_thread_t thread_3;
    static fx_thread_t thread_4;
    static uint32_t stack_0[0x200 / sizeof(uint32_t)];
    static uint32_t stack_1[0x200 / sizeof(uint32_t)];
    static uint32_t stack_2[0x200 / sizeof(uint32_t)];
    static uint32_t stack_3[0x200 / sizeof(uint32_t)];
    static uint32_t stack_4[0x200 / sizeof(uint32_t)];
    //
    // Initializing threads and rwlock.
    //
    fx_rwlock_init(&rwlock, FX_SYNC_POLICY_DEFAULT);
    fx_thread_init(&thread_0, buf_write, "0", 10, stack_0, sizeof(stack_0), 0);
    fx_thread_init(&thread_1, buf_write, "1", 10, stack_1, sizeof(stack_1), 0);
    fx_thread_init(&thread_2, buf_read, "2", 10, stack_2, sizeof(stack_2), 0);
    fx_thread_init(&thread_3, buf_read, "3", 10, stack_3, sizeof(stack_3), 0);
    fx_thread_init(&thread_4, buf_read, "4", 10, stack_4, sizeof(stack_4), 0);
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
