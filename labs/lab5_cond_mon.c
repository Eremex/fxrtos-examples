/*
 *  Copyright (C) JSC EREMEX, 2019-2022.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  FX-RTOS course thread lab.
 *  Assignment:
 *  1) Three equal counting threads given
 *  2) Implement barrier function for synchronous threads finish
 *
*/

#include <stdio.h>
#include <stdlib.h>

#include <FXRTOS.h>
#include "demo_bsp.h"

#define SET_ROUND_ROBIN_TIMESLICE   0

#define MAX_WORKERS	3
unsigned sum[MAX_WORKERS];
//
// Use this variables:
//
size_t num_arrived;
fx_mutex_t barrier_mtx;
fx_cond_t go_cond;

//
// Imitation of cpu load slowdown and spontaneous preemption
//
static void load_cpu_for_time()
{
    for (unsigned i = 0; i < 500; i++)
    {
    	;
    }
    if ((rand() % 2) == 0)
    {
        fx_thread_yield();
    }
}

//
// TODO: implement monitor "barrier"
//
static void barrier(void)
{
	;
}

void thread_worker(void *arg)
{
	unsigned my_id = (unsigned)arg;
    printf("Worker thread %u started\r\n", my_id);

    for (unsigned i = 0; i < 1000; i++)
    {
    	sum[my_id]++;
    	load_cpu_for_time();
    }

    printf("Worker thread %u come to barrier\r\n", my_id);
    barrier();
    printf("Worker thread %u passed barrier\r\n", my_id);

    if (my_id == 0)
    {
    	unsigned total_sum = 0;
    	for (unsigned worker_id = 0; worker_id < MAX_WORKERS; worker_id++)
    	{
    		total_sum += sum[worker_id];
    	}
        printf("Total sum = %u\r\n", total_sum);
        led_on();
    }

    fx_thread_suspend();
}

void fx_intr_handler(void)
{
    ;
}

void fx_app_init(void)
{
    static fx_thread_t t[MAX_WORKERS];
    static int thread_stk[MAX_WORKERS][2548 / sizeof(int)];

    //
    // TODO: Put primitives initialization here
    //
	fx_cond_init(&go_cond, FX_SYNC_POLICY_FIFO);
    fx_mutex_init(&barrier_mtx, FX_MUTEX_CEILING_DISABLED, FX_SYNC_POLICY_FIFO);

    fx_sched_state_t prev;
    fx_sched_lock(&prev);

    for (unsigned id = 0; id < MAX_WORKERS; id++)
    {
    	fx_thread_init(&t[id], thread_worker, (void*)id, 10, thread_stk[id],
            sizeof(thread_stk[0]), false);
#if SET_ROUND_ROBIN_TIMESLICE == 1
    	unsigned int timeslice = 100;
    	fx_thread_set_params(&t[worker_id], FX_THREAD_PARAM_TIMESLICE, timeslice);
#endif
    }

    fx_sched_unlock(prev);
}

//!
//! Application entry point
//!
int main(void)
{
    //
    // Hardware modules initialization
    //
    core_init();
    led_init();
    console_init();

    //
    // Kernel start. This function must be called with interrupts disabled
    //
    fx_kernel_entry();
}
