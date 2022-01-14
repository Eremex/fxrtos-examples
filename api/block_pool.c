#include "demo_bsp.h"
#include <FXRTOS.h>
#include <stdio.h>
#include <string.h>

fx_block_pool_t pool;
// In this program 2 blocks of size 100 are used, but their real size is 
// 100 + sizeof(uintptr_t), so it must be taken into account when setting the 
// pool size.
uint32_t buf[208 / sizeof(uint32_t)];


extern void led_on(void);
extern void led_off(void);

// Task_0 allocates all free blocks from pool, so thread_1 go to wait when
// trying to allocate more blocks.
void
task_0(void* args)
{
    uint32_t* chunk_0;
    uint32_t* chunk_1;
    unsigned free_blocks;
    printf("Thread_0 started\n\r");

    fx_block_pool_avail_blocks(&pool, &free_blocks);
    printf("There are %u free chunks in pool\n\r", free_blocks);

    fx_block_pool_alloc(&pool, &chunk_0, NULL);
    fx_block_pool_alloc(&pool, &chunk_1, NULL);
    printf("Thread_0 allocated 2 chunks\n\r");

    fx_thread_sleep(1000);

    fx_block_pool_release(chunk_0);
    printf("Thread_0 released chunk_0\n\r");

    fx_thread_sleep(1000);

    fx_block_pool_release(chunk_1);
    printf("Thread_0 released chunk_1\n\r");

    fx_thread_exit();
}

// Thread_1 trying to allocate blocks, when there are no free blocks in pool, so
// it goes to wait for their release.
void
task_1(void* args)
{
    uint32_t* chunk_0;
    uint32_t* chunk_1;
    printf("Thread_1 started\n\r");

    printf("Thread_1 going to allocate chunk_0\n\r");
    fx_block_pool_alloc(&pool, &chunk_0, NULL);
    printf("Thread_1 allocated chunk_0\n\r");

    printf("Thread_1 going to allocate chunk_1\n\r");
    fx_block_pool_alloc(&pool, &chunk_1, NULL);
    printf("Thread_1 allocated chunk_1\n\r");

    fx_block_pool_release(chunk_0);
    fx_block_pool_release(chunk_1);

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

    printf("INIT %u\n\r", sizeof(buf));
    // Initializing threads and block pool.
    fx_block_pool_init(&pool, buf, sizeof(buf), 100, FX_SYNC_POLICY_FIFO);
    fx_thread_init(&thread_0, task_0, "0", 10, stack_0, sizeof(stack_0), 0);
    fx_thread_init(&thread_1, task_1, "1", 10, stack_1, sizeof(stack_1), 0);
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
