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
 *  Create LED blinking application.
 *  Organize equal time sharing between two threads different ways:
 *  1) Round-robin scheduler with preemptive multitasking (setting time slice)
 *  2) Use FIFO scheduling and cooperative multitasking (fx_thread_yield)
 *  3) Use blocking system calls (fx_thread_sleep)
 *  4) Set different priorities and watch result.
 *
 */

#include <stdio.h>

#include <FXRTOS.h>
#include "demo_bsp.h"

#define SET_ROUND_ROBIN_TIMESLICE   0

static void load_cpu_for_time(void (*func)(void))
{
    for (unsigned i = 0; i < 100000; i++)
    {
        func();
    }
}

void thread_1_fn(void *arg)
{
    printf("Thread 1 started\r\n");

    while (1)
    {
        load_cpu_for_time(led_off);
    }
}

void thread_0_fn(void *arg)
{
    printf("Thread 0 started\r\n");

    while (1)
    {
        load_cpu_for_time(led_on);
    }
}

void fx_intr_handler(void)
{
    ;
}

void fx_app_init(void)
{
    static fx_thread_t t[2];
    static int thread_0_stk[1024 / sizeof(int)];
    static int thread_1_stk[1024 / sizeof(int)];

    fx_sched_state_t prev;
    fx_sched_lock(&prev);

    fx_thread_init(&t[0], thread_0_fn, NULL, 10, thread_0_stk,
            sizeof(thread_0_stk), false);

    fx_thread_init(&t[1], thread_1_fn, NULL, 10, thread_1_stk,
            sizeof(thread_1_stk), false);

#if SET_ROUND_ROBIN_TIMESLICE == 1
    unsigned int timeslice = 100;
    fx_thread_set_params(&t[0], FX_THREAD_PARAM_TIMESLICE, timeslice);
    fx_thread_set_params(&t[1], FX_THREAD_PARAM_TIMESLICE, timeslice);
#endif

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
