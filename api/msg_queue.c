#include "demo_bsp.h"
#include <FXRTOS.h>
#include <stdio.h>
#include <string.h>

fx_msgq_t queue;
uintptr_t buf[10];


extern void led_on(void);
extern void led_off(void);

// This function demonstrates how to use msgq as data structure.
void
msgq_as_data_struct(void)
{
    uint32_t msg;

    printf("Using message queue as queue\n\r");
    for (uint32_t i = 0; i < 10; i++)
    {
        fx_msgq_back_send(&queue, i, NULL);
    }
    while (!fx_msgq_timedreceive(&queue, &msg, 1))
    {
        printf("%lu ", msg);
    }
    printf("\n\n\r");

    printf("Using message queue as stack\n\r");
    for (uint32_t i = 0; i < 10; i++)
    {
        fx_msgq_front_send(&queue, i, NULL);
    }
    while (!fx_msgq_timedreceive(&queue, &msg, 1))
    {
        printf("%lu ", msg);
    }
    printf("\n\r");
}

// This task works in pair with task_1 and demonstrates how to use msgq as
// sync object.
void
task_0(void* args)
{
    uintptr_t msg;
    printf("Thread_0 started\n\r");
    
    printf("Thread_0 going to wait msg\n\r");
    fx_msgq_receive(&queue, &msg, NULL);
    printf("Thread_0 received msg\n\n\r");

    for (unsigned i = 0; i < 10; i++)
    {
        fx_msgq_back_send(&queue, 0, NULL);
    }

    printf("Thread_0 going to wait free space in queue\n\r");
    fx_msgq_back_send(&queue, 0, NULL);
    printf("Thread_0 sent message\n\n\r");

    // Clearing queue for next actions.
    fx_msgq_flush(&queue);

    msgq_as_data_struct();
    fx_thread_exit();
}


void
task_1(void* args)
{
    uintptr_t msg;
    printf("Thread_1 started\n\r");

    printf("Thread_1 going to send msg\n\r");
    fx_msgq_back_send(&queue, 0, NULL);
    printf("Thread_1 sent msg\n\r");
    
    fx_thread_yield();

    printf("Thread_1 going to receive msg\n\r");
    fx_msgq_receive(&queue, &msg, NULL);
    printf("Thread_1 received msg\n\r");

    fx_thread_exit();
}

void
fx_app_init(void)
{
    // Creating threads and their stacks.
    static fx_thread_t thread_0;
    static fx_thread_t thread_1;
    static uint32_t stack_0[0x200 / sizeof(uint32_t)];
    static uint32_t stack_1[0x200 / sizeof(uint32_t)];

    // Initializing threads and message queue.
    fx_msgq_init(&queue, buf, 10, FX_SYNC_POLICY_FIFO);
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
