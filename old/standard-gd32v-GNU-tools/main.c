/*
 *  Copyright (C) JSC EREMEX, 2020.
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
 *  FX-RTOS demo application.
 *  The demo creates three LED-blinking threads.
 */

#include "FXRTOS.h"
#include <system_gd32vf103.h>

#define LED_R_PIN 13
#define LED_G_PIN 1
#define LED_B_PIN 2

static fx_sem_t sem1;
static fx_sem_t sem2;
static fx_sem_t sem3;
int main_stack[0x200 / sizeof(int)];
static volatile uint32_t* const gpioa_ctl0 = (uint32_t*)0x40010800;
static volatile uint32_t* const gpioa_out_ctl = (uint32_t*)0x4001080C;
static volatile uint32_t* const gpioc_ctl1 = (volatile uint32_t*)0x40011004;
static volatile uint32_t* const gpioc_out_ctl = (volatile uint32_t*)0x4001100C;
static volatile uint64_t* const mtimecmp = (volatile uint64_t*)0xd1000008;
static volatile uint32_t* const rcu_apb2en = (volatile uint32_t*)0x40021018;

extern void systick_enable(void);
extern void systick_disable(void);

//
// If HAL_CPU_INTR = RV32I_V2
//
void hal_pic_msip_set(unsigned int req)
{
    static volatile uint32_t* const msip = (volatile uint32_t*) 0xd1000ffc;
    *msip = req;
}

static inline void mtimer_setup(void)
{
    uint64_t temp = *mtimecmp;
    temp += (SystemCoreClock/4)/1000;
    *mtimecmp = temp;
}

extern void mtimer_int_enable();
extern void mtimer_int_disable();

void hal_timer_pre_tick(void)
{
    mtimer_int_disable();
}

void hal_timer_post_tick(void)
{
    mtimer_setup();
    mtimer_int_enable();
}

static void green(bool off_on)
{
    uint32_t v = *gpioa_ctl0;
    v &= ~(0xF << LED_G_PIN*4);
    v |= (0x3 << LED_G_PIN*4);
    *gpioa_ctl0 = v;
     
    v = *gpioa_out_ctl;
    v &= ~(1 << LED_G_PIN); // clear bit
    v |= (~off_on << LED_G_PIN); // set value inverted
    *gpioa_out_ctl = v;
}

static void blue(bool off_on)
{
    uint32_t v = *gpioa_ctl0;
    v &= ~(0xF << LED_B_PIN*4);
    v |= (0x3 << LED_B_PIN*4);
    *gpioa_ctl0 = v;

    v = *gpioa_out_ctl;
    v &= ~(1 << LED_B_PIN); // clear bit
    v |= (~off_on << LED_B_PIN); // set value inverted
    *gpioa_out_ctl = v;
}

static void red(bool off_on)
{
    uint32_t v = *gpioc_ctl1;
    v &= ~(0xF << 20);
    v |= (0x3 << 20);
    *gpioc_ctl1 = v;

    v = *gpioc_out_ctl; // save state
    v &= ~(1 << LED_R_PIN); // clear bit
    v |= (~off_on << LED_R_PIN); // set value inverted
    *gpioc_out_ctl = v;
}

//
// Simulate fade effect with pulse width modulation.
// dir parameter must be either 1 for "fade in" or -1 for "fade out".
// Duration must be divisable by 10.
//
static void fade(void(*ctrl)(bool), const unsigned int duration, const signed int dir)
{
    const unsigned int T = 10;
    const unsigned int n = duration / T;
    unsigned int s = T * (unsigned int)(dir > 0);
    unsigned int m = 0;
    unsigned int i = 0;

    for (i = 0; i < n; ++i)
    {
        const unsigned int f = (i * T) / n;
        s += ((int)(m != f)) * -dir;
        m = f;

        ctrl(false); // off
        fx_thread_sleep(s);
        ctrl(true); // on
        fx_thread_sleep(T - s);  
    }
}


void thread1_fn(void* arg)
{
    (void)arg;
    fx_sem_post(&sem1);
    while (1)
    {
        
        fx_sem_wait(&sem1, NULL);
        fade(green, 600, 1);
        fade(green, 600, -1);
        green(false);
        fx_sem_post(&sem2);
        
    }
}

void thread2_fn(void* arg)
{
    (void)arg;
    
    while (1)
    {
        fx_sem_wait(&sem2, NULL);
        fade(blue, 600, 1);
        fade(blue, 600, -1);
        blue(false);
        fx_sem_post(&sem3);
    }
}

void thread3_fn(void* arg)
{
    (void)arg;
    while (1)
    { 
        fx_sem_wait(&sem3, NULL);
        fade(red, 600, 1);
        fade(red, 600, -1);
        red(false);
        fx_sem_post(&sem1);
    }
}

/* Application definition. Called by the kernel after initialization. */

void fx_app_init(void)
{
    static fx_thread_t t1;
    static fx_thread_t t2;
    static fx_thread_t t3;
    static int t1_stk[0x100];
    static int t2_stk[0x100];
    static int t3_stk[0x100];

    *rcu_apb2en |= (1 << 4); // GPIOC enable
    *rcu_apb2en |= (1 << 2); // GPIOA enable
    green(false);
    blue(false);
    red(false);


    mtimer_setup();
    mtimer_int_enable();
    fx_sem_init(&sem1, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_sem_init(&sem2, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_sem_init(&sem3, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_thread_init(&t1, thread1_fn, NULL, 5, t1_stk, sizeof(t1_stk), false); 
    fx_thread_init(&t2, thread2_fn, NULL, 5, t2_stk, sizeof(t2_stk), false);
    fx_thread_init(&t3, thread3_fn, NULL, 5, t3_stk, sizeof(t2_stk), false); 
}

/* Program entry point. Interrupts are disabled by startup code. */

extern char _bss;
extern char _ebss;

int main(void)
{
       
    SystemInit();

    fx_kernel_entry();
    return 0;
}

/* No interrupts except timer are allowed. */

void fx_intr_handler(void)
{
    ;
}

/* We do not handle exceptions! */

void hal_trap_handler(void)
{
    for (;;) ;
}
