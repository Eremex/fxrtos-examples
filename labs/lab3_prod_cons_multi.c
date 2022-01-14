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
 *  FX-RTOS course thread syncronization lab.
 *  Assignment:
 *  1) Two threads produces text messages, one prints
 *  2) Create message queue with methods: init, put, get
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <FXRTOS.h>

#define SET_ROUND_ROBIN_TIMESLICE   0

extern void led_on();
extern void led_off();

static void load_cpu_for_time1(void (*func)(void))
{
    for (unsigned i = 0; i < 1000; i++)
    {
        func();
    }
    if ((rand() % 2) != 0)
    {
    	fx_thread_yield();
    }
}

static void load_cpu_for_time2(void (*func)(void))
{
    for (unsigned i = 0; i < 3300; i++)
    {
        func();
    }
    if ((rand() % 2) != 0)
    {
    	fx_thread_yield();
    }
}

#define VECT_SZ		3
#define MSG_SZ_MAX	64

typedef struct queue_s
{
	fx_sem_t full;
	fx_sem_t empty;
	fx_mutex_t mutex;
	unsigned put_idx;
	unsigned get_idx;
	uint8_t msg_buf[VECT_SZ][MSG_SZ_MAX];
	size_t msg_size[VECT_SZ];
} queue_t;

static queue_t msg_queue;
static queue_t* q = &msg_queue;

static void queue_init(queue_t *q)
{
    // TODO: queue initialization here
}

static void queue_put(queue_t* q, void* p, size_t size)
{
    // TODO: add item to the queue
}

static size_t queue_get(queue_t* q, void* p)
{
    size_t msg_size;
    // TODO: get item from the queue
    return msg_size;
}

void thread_0_fn(void *arg)
{
    printf("Thread 0 started\r\n");

    char msg[MSG_SZ_MAX];
    unsigned long counter = 0;
    while (1)
    {
    	load_cpu_for_time1(led_on);
        sprintf(msg, "Hello from thread 0 %lu!\r\n", counter++);
        queue_put(q, msg, strlen(msg)+1);
    }
}

void thread_1_fn(void *arg)
{
    printf("Thread 1 started\r\n");

    char msg[MSG_SZ_MAX];
    unsigned long counter = 0;
    while (1)
    {
    	load_cpu_for_time2(led_off);
        sprintf(msg, "Hello from thread 1 %lu!\r\n", counter++);
        queue_put(q, msg, strlen(msg)+1);
    }
}

void thread_2_fn(void *arg)
{
    printf("Thread 2 consumer started\r\n");
    char msg[MSG_SZ_MAX];
    size_t size;
    while (1)
    {
        load_cpu_for_time2(led_off);
    	size = queue_get(q, msg);
    	printf("received %u bytes: %s\n", size, msg);
    }
}

void fx_app_init(void)
{
    static fx_thread_t t[3];
    static int thread_0_stk[2548 / sizeof(int)];
    static int thread_1_stk[2548 / sizeof(int)];
    static int thread_2_stk[2548 / sizeof(int)];

    queue_init(q);

    fx_sched_state_t prev;
    fx_sched_lock(&prev);

    fx_thread_init(&t[0], thread_0_fn, NULL, 10, thread_0_stk,
            sizeof(thread_0_stk), false);

    fx_thread_init(&t[1], thread_1_fn, NULL, 10, thread_1_stk,
            sizeof(thread_1_stk), false);

    fx_thread_init(&t[2], thread_2_fn, NULL, 10, thread_2_stk,
               sizeof(thread_2_stk), false);

#if SET_ROUND_ROBIN_TIMESLICE == 1
    unsigned int timeslice = 100;
    fx_thread_set_params(&t[0], FX_THREAD_PARAM_TIMESLICE, timeslice);
    fx_thread_set_params(&t[1], FX_THREAD_PARAM_TIMESLICE, timeslice);
    fx_thread_set_params(&t[2], FX_THREAD_PARAM_TIMESLICE, timeslice);
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

