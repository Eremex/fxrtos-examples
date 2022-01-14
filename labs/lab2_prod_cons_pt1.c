/*
 *  Copyright (C) JSC EREMEX, 2008-2020.
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
 *  1) Create thread producing text messages
 *  2) Create consumer thread which prints this messages
 *  3) Use full_sem to signal about new message in buffer
 *  4) Add empty_sem to signal about free buffer to avoid rewriting
 *
 *  API example:
 *  fx_sem_t sem;
*   fx_sem_init(&sem, init_val, max_val, FX_SYNC_POLICY_FIFO);
*   fx_sem_timedwait(&sem, FX_THREAD_INFINITE_TIMEOUT);
*   fx_sem_post(&sem);
*
*/

#include <stdio.h>
#include <string.h>

#include <FXRTOS.h>

#define SET_ROUND_ROBIN_TIMESLICE   1

extern void led_on();
extern void led_off();

#define MSG_SIZE	64
char msg_buf[MSG_SIZE];

static unsigned long counter;
// TODO semaphore declaration here

static void load_cpu_for_time1(void (*func)(void))
{
    for (unsigned i = 0; i < 10000; i++)
    {
        func();
    }
}

static void load_cpu_for_time2(void (*func)(void))
{
    for (unsigned i = 0; i < 33000; i++)
    {
        func();
    }
}

void produce(char* buf)
{
    load_cpu_for_time1(led_on);
    sprintf(buf, "Hello from thread 0 %lu!\r\n", counter++);
}

void consume(char* buf)
{
    load_cpu_for_time2(led_off);
    printf("t1 received: %s", msg_buf);
    memset(buf, 0, MSG_SIZE);
}

//
// Producer
//
void thread_0_fn(void *arg)
{
    printf("Thread 0 started\r\n");

    unsigned long counter = 0;
    while (1)
    {
	// TODO add syncronization
        produce(msg_buf);
	// TODO add syncronization
    }
}

//
// Consumer
// TODO add syncronization
//
void thread_1_fn(void *arg)
{
    printf("Thread consumer started\r\n");

    while (1)
    {
        // TODO add syncronization
        consume(msg_buf);
        // TODO add syncronization
    }
}


void fx_app_init(void)
{
    static fx_thread_t t[2];
    static int thread_0_stk[2048 / sizeof(int)];
    static int thread_1_stk[2048 / sizeof(int)];

    // TODO semaphore initialization here

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

void fx_intr_handler(void)
{
    ;
}

//
// Exception handler just stop the system.
//
void system_stop(void)
{
    while(1)
        ;
}
