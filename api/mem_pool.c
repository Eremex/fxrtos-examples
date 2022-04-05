#include <stdio.h>
#include <FXRTOS.h>
#include "demo_bsp.h"

fx_mem_pool_t pool;
uint32_t buf[416 / sizeof(uint32_t)];

//
// This task demonstrates, that only contiguous block of memory can be allocated
// from pool.
// After each action there is schematic view of current pool state, where each
// sign is ~50 bytes and 
// 0 means that this 50 bytes are allocated for chunk_0
// 1 means that this 50 bytes are allocated for chunk_1
// . means that tis 50 bytes are free
//
void
task(void* args)
{
    size_t free_chunk_size;
    uint32_t* chunk_0;
    uint32_t* chunk_1;

    fx_mem_pool_add_mem(&pool, buf, sizeof(buf));
    fx_mem_pool_get_max_free_chunk(&pool, &free_chunk_size);
    printf("Free space at start: %u\n\r", free_chunk_size);
    //
    // Pool state: [........]
    //
    fx_mem_pool_alloc(&pool, 100, &chunk_0);
    fx_mem_pool_alloc(&pool, 100, &chunk_1);

    fx_mem_pool_get_max_free_chunk(&pool, &free_chunk_size);
    printf("Free space after 2 chunks allocated: %u\n\r", free_chunk_size);
    //
    // Pool state: [0011....]
    //
    fx_mem_pool_free(&pool, chunk_0);

    fx_mem_pool_get_max_free_chunk(&pool, &free_chunk_size);
    printf("Free space after chunk_0 released: %u\n\r", free_chunk_size);
    //
    // Pool state: [..11....]
    //
    fx_mem_pool_alloc(&pool, 150, &chunk_0);

    fx_mem_pool_get_max_free_chunk(&pool, &free_chunk_size);
    printf("Free space after chunk_0 reallocated: %u\n\r", free_chunk_size);
    //
    // Pool state: [..11000.]
    //
    fx_mem_pool_free(&pool, chunk_0);
    fx_mem_pool_free(&pool, chunk_1);

    fx_mem_pool_get_max_free_chunk(&pool, &free_chunk_size);
    printf("Free space after chunks released: %u\n\r", free_chunk_size);
    //
    // Pool state: [........]
    //
    fx_mem_pool_deinit(&pool);

    fx_thread_exit();
}

void
fx_app_init(void)
{
    //
    // Creating threads and their stacks.
    //
    static fx_thread_t thread;
    static uint32_t stack[0x200 / sizeof(uint32_t)];
    //
    // Initializing thread and memory pool.
    //
    fx_mem_pool_init(&pool);
    fx_thread_init(&thread, task, NULL, 10, stack, sizeof(stack), 0);
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
