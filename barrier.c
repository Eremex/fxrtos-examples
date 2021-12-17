#include "demo_bsp.h"
#include <FXRTOS.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

fx_barrier_t barrier;

extern void led_on(void);
extern void led_off(void);

// This task demonstrates how to determine thread which opened barrier.
void
task(void* args)
{
    fx_barrier_key_t key;
    for (;;)
    {
        fx_thread_sleep(100 + rand() % 900);
        fx_barrier_wait(&barrier, &key, NULL);
        printf("Thread_%c is %s\n\r", *(char*)args,
               key == FX_BARRIER_SERIAL_THREAD ? "opener" : "regular");
    }
}

void
fx_app_init(void)
{
    // Creating threads and their stacks.
    static fx_thread_t thread_0;
    static fx_thread_t thread_1;
    static fx_thread_t thread_2;
    static fx_thread_t thread_3;
    static uint32_t stack_0[0x200 / sizeof(uint32_t)];
    static uint32_t stack_1[0x200 / sizeof(uint32_t)];
    static uint32_t stack_2[0x200 / sizeof(uint32_t)];
    static uint32_t stack_3[0x200 / sizeof(uint32_t)];

    srand(time(NULL));
    // Initializing threads and barrier.
    fx_barrier_init(&barrier, 4);
    fx_thread_init(&thread_0, task, "0", 10, stack_0, sizeof(stack_0), 0);
    fx_thread_init(&thread_1, task, "1", 10, stack_1, sizeof(stack_1), 0);
    fx_thread_init(&thread_2, task, "2", 10, stack_2, sizeof(stack_2), 0);
    fx_thread_init(&thread_3, task, "3", 10, stack_3, sizeof(stack_3), 0);
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
