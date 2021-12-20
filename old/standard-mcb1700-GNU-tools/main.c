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
 *  FX-RTOS demo application.
 *
 *  This demo demonstrates how to use threads and interrupts on FX-RTOS.
 *  Application creates two threads: LED thread and RIT thread, first one is 
 *  used to toggle LED 0 using standard sleep API to make delays. Second one is 
 *  signaled from an interrupt handler. Repetitive Interrupt Timer (RIT) is used
 *  as periodic interrupt source.
 */

#include "FXRTOS.h"
#include "LPC17xx.h"

void hw_cpu_intr_enable(void);
void led_init(void);
void led_on(int num);
void led_off(int num);

//
// Semaphore for interrupt-to-thread synchronization.
//
static fx_sem_t sem;
static fx_dpc_t dpc;

static void dpc_handler(fx_dpc_t* dpc, void* arg)
{
    //
    // Post semaphore and wake the waiting thread up.
    //
    fx_sem_post(&sem);
}

//
// This function is called by the kernel as a result of any interrupt.
// HAL or CPU interfaces should be used in order to distinct one vector from 
// another. This demo uses HAL function to obtain the vector.
//
void fx_intr_handler(void)
{
    unsigned int vector = hal_intr_get_current_vect();

    //
    // Check whether the vector is ours...
    //
    switch (vector)
    {
    case  RIT_IRQn:

        fx_dpc_request(&dpc, dpc_handler, NULL);

        //
        // Send End-Of-Interrupt signal to the device.
        //
        LPC_RIT->RICTRL |= 1;
        break;

    default: break;
    }
}

//
// LED-linking thread.
//
static void led_thread_fn(void* arg)
{
    while(1)
    {
        fx_thread_sleep(500);
        led_on(0);
        fx_thread_sleep(500);
        led_off(0);
    }
}

//
// Interrupt handling thread.
//
static void rit_thread_fn(void* arg)
{
    bool set = false;

    while(1)
    {
        //
        // Wait for signal from the interrupt handler.
        //
        fx_sem_timedwait(&sem, FX_THREAD_INFINITE_TIMEOUT);

        //
        // Toggle LED.
        //
        set = !set;

        if (set)
            led_on(1);
        else 
            led_off(1);
    }
}

//
// Application definition function.
//
void fx_app_init(void)
{
    //
    // Threads and stacks allocation.
    //
    static fx_thread_t thread_led;
    static int thread_led_stk[0x100];
    static fx_thread_t thread_rit;
    static int thread_rit_stk[0x100];

    hw_cpu_intr_enable();

    //
    // Initialize semaphore object.
    // This must be done before it is used in interrupt handler!
    //
    fx_sem_init(&sem, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_dpc_init(&dpc);

    //
    // Power-on, program the RIT and enable its IRQ line...
    //
    LPC_SC->PCONP |= (1L<<16); 
    LPC_SC->PCLKSEL1 &= ~(3L << 26); 
    LPC_SC->PCLKSEL1 |=  (1L << 26);
    LPC_RIT->RICOMPVAL = 10000000;
    LPC_RIT->RICOUNTER = 0;
    LPC_RIT->RICTRL = (1 << 3) | (1 << 2) | (1 << 1);
    NVIC_EnableIRQ(RIT_IRQn);

    //
    // Init and start both threads.
    //
    fx_thread_init(
        &thread_led, 
        led_thread_fn, 
        NULL, 
        5, 
        thread_led_stk, 
        sizeof(thread_led_stk), 
        false
    ); 

    fx_thread_init(
        &thread_rit, 
        rit_thread_fn, 
        NULL, 
        5, 
        thread_rit_stk, 
        sizeof(thread_rit_stk), 
        false
    ); 
}

//
// Exception handler just stop the system and sets LED7.
//
void system_stop(void)
{ 
    led_on(2);
    while(1); 
}
    
//
// Application entry point.
//
int main(void)
{
    //
    // Initialize clocks, PLL, LEDs, etc...
    //
    SystemInit();
    SystemCoreClockUpdate();
    led_init();

    //
    // Setup PendSV and SysTick prioritiy at lowest possible level.
    // It is required for proper kernel initialization.
    //
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
    NVIC_SetPriority(SysTick_IRQn, 0xFF);
    NVIC_SetPriorityGrouping(2);

    //
    // Start system tick source.
    //
    SysTick->LOAD  = 100000 - 1;
    SysTick->VAL   = 0;
    SysTick->CTRL  = (1 << 2) | (1 << 0) | (1 << 1);

    //
    // Start scheduling loop.
    //
    fx_kernel_entry();
    return 0;
}
