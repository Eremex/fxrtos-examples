#include "demo_bsp.h"
#include <FXRTOS.h>
#include <stdio.h>
#include <string.h>
#include "demo_bsp.h"


fx_dpc_t dpc;
fx_sem_t semaphore;


extern void led_on(void);
extern void led_off(void);

void
task(void* args)
{
    for (;;)
    {
    	printf("Going to wait for interrupt\n\r");
        fx_sem_wait(&semaphore, NULL);
        printf("Semaphore opened\n\r");
        led_on();
        fx_thread_sleep(100);
        led_off();
    }
    fx_thread_exit();
}

// DPC function. It will be called after main interrupt handler.
void
tim_dpc(fx_dpc_t* dpc, void* args)
{
    fx_sem_post(&semaphore);
}

// Main interrupt handler. Here should be done the most important things related
// to interrupt handling. Other work should be done in dpc function.
void
tim_handler(void)
{
	timer_restart();
	fx_dpc_request(&dpc, tim_dpc, NULL);
}

void
fx_app_init(void)
{
    // Creating thread and its stack.
    static fx_thread_t thread;
    static uint32_t stack[0x200 / sizeof(uint32_t)];

    // Initializing thread, semaphore and dpc.
    fx_dpc_init(&dpc);
    fx_sem_init(&semaphore, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_thread_init(&thread, task, NULL, 10, stack, sizeof(stack), 0);
}

// Interrupt handler selector.
void
fx_intr_handler(void)
{
    switch (hal_intr_get_current_vect())
    {
        case timer_irq:
            tim_handler();
            break;
        default:
            break;
    }
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
