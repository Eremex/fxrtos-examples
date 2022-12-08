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
 *  1) Three treads with different priorities.
 *  2) Use priority ceiling to resolve priority inversion problem
 *  3) Goal: The most priority thread must to enter critical section (and turn on LED) as soon as possible.
 *  4) You cannot change the threads functions code.
 *
*/

#include <stdio.h>
#include <string.h>

#include <FXRTOS.h>
#include "demo_bsp.h"

#define SET_ROUND_ROBIN_TIMESLICE   0

fx_mutex_t mutex;

static void load_cpu_for_time(void (*func)(void))
{
    for (unsigned i = 0; i < 1000000; i++)
    {
        func();
    }
}

void thread_0_low_prio(void *arg)
{
    printf("T0 (low priority) started\r\n");

    fx_mutex_timedacquire(&mutex, FX_THREAD_INFINITE_TIMEOUT);

    printf("T0 (low priority) enter\r\n");
    load_cpu_for_time(led_off);
    printf("T0 (low priority) leave\r\n");

    fx_mutex_release(&mutex);

    fx_thread_exit();
}

void thread_1_medium_prio(void *arg)
{
    fx_thread_sleep(100);
    printf("T1 (medium priority) started\r\n");

    load_cpu_for_time(led_off);

    fx_thread_exit();
}

void thread_2_high_prio(void *arg)
{
    fx_thread_sleep(200);
    printf("T2 (high priority) started\r\n");

    fx_mutex_timedacquire(&mutex, FX_THREAD_INFINITE_TIMEOUT);

    printf("T2 (high priority) enter\r\n");
    load_cpu_for_time(led_on);
    printf("T2 (high priority) leave\r\n");

    fx_mutex_release(&mutex);

    fx_thread_exit();
}

void fx_intr_handler(void)
{
    ;
}

void fx_app_init(void)
{
    static fx_thread_t t[3];
    static int thread_0_stk[2548 / sizeof(int)];
    static int thread_1_stk[2548 / sizeof(int)];
    static int thread_2_stk[2548 / sizeof(int)];
    printf("================\r\n");
    fx_mutex_init(&mutex, FX_MUTEX_CEILING_DISABLED, FX_SYNC_POLICY_FIFO);

    fx_sched_state_t prev;
    fx_sched_lock(&prev);

    fx_thread_init(&t[0], thread_0_low_prio, NULL, 10, thread_0_stk,
            sizeof(thread_0_stk), false);

    fx_thread_init(&t[1], thread_1_medium_prio, NULL, 9, thread_1_stk,
            sizeof(thread_1_stk), false);

    fx_thread_init(&t[2], thread_2_high_prio, NULL, 8, thread_2_stk,
               sizeof(thread_2_stk), false);

#if SET_ROUND_ROBIN_TIMESLICE == 1
    unsigned int timeslice = 100;
    fx_thread_set_params(&t[0], FX_THREAD_PARAM_TIMESLICE, timeslice);
    fx_thread_set_params(&t[1], FX_THREAD_PARAM_TIMESLICE, timeslice);
    fx_thread_set_params(&t[2], FX_THREAD_PARAM_TIMESLICE, timeslice);
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
