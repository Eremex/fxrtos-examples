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
 *  FX-RTOS demo application for STM32 Blue Pill board.
 *  This demo uses internal timer and ISR-to-thread communication to toggle LED.
 *
 *  IMPORTANT!
 *  IN THIS DEMO WE USE CUSTOM THREAD WITH LOWEST PRIORITY AS IDLE THREAD.
 *  THIS IS BECAUSE OS' INTERNAL IDLE THREAD SWITCHES CPU TO LOW POWER MODE BY
 *  USING WFI INSTRUCTION.
 *  SOME DEBUGGERS LIKE STLINK-V2 CANNOT TALK TO CPU WHICH IS IN LOW POWER MODE,
 *  SO, TO SIMPLIFY DEBUGGER USE WE USE ALWAYS-READY THREAD WITH INFINITE CYCLE
 *  TO PREEMPT INTERNAL IDLE THREAD.
 *  IN CASE WHEN CPU UNINTENTIONALLY FLASHED WITH CODE THAT PUTS IT INTO LOW
 *  POWER MODE YOU CAN USE FOLLOWING REPROGRAMMING SEQUENCE:
 *  1) PRESS RESET BUTTON ON BOARD
 *  2) START PROGRAMMER (ST-LINK_CLI.exe -P demo.bin 0x08000000 -Rst -Run)
 *  3) RELEASE RESET BUTTON
 */

#include "FXRTOS.h"
#include "system_stm32f1xx.h"
#include "stm32f1xx.h"

//
// Semaphore for interrupt-to-thread synchronization.
//
static fx_sem_t sem;
static fx_dpc_t dpc;

//
// Simulate fade effect with pulse width modulation.
// dir parameter must be either 1 for "fade in" or -1 for "fade out".
// Duration must be divisable by 10.
//
static void fade(const unsigned int duration, const signed int dir)
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

        GPIOC->BSRR = GPIO_BSRR_BS13; // off
        fx_thread_sleep(s);
        GPIOC->BSRR = GPIO_BSRR_BR13; // on
        fx_thread_sleep(T - s);  
    }
}

//
// Deferred interrupt processing function.
//
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
    const unsigned int vector = hal_intr_get_current_vect();

    //
    // Check whether the vector is ours...
    //
    if (vector == TIM2_IRQn)
    {
        TIM2->SR &= ~TIM_SR_UIF;
        fx_dpc_request(&dpc, dpc_handler, NULL);
    }
}

//
// LED-blinking thread. It waits for Timer interrupt and switches the LED.
//
static void led_thread_fn(void* arg)
{
    while (1)
    {   
        GPIOC->BSRR = GPIO_BSRR_BS13;
        fx_sem_timedwait(&sem, FX_THREAD_INFINITE_TIMEOUT);
        fade(300, 1);
        fade(300, -1);
        fade(300, 1);
        fade(300, -1);        
    }
}

//
// Custom idle thread with infinite cycle.
//
static void idle_thread_fn(void* arg)
{
    while (1)
    {
        ;
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
    static fx_thread_t thread_idle;
    static int thread_led_stk[0x100];
    static int thread_idle_stk[0x100];

    hw_cpu_intr_enable();

    //
    // Initialize semaphore object.
    // This must be done before it is used in interrupt handler!
    //
    fx_sem_init(&sem, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_dpc_init(&dpc);

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH |= GPIO_CRH_CNF13_0 | GPIO_CRH_MODE13_1;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
    
    TIM2->PSC = 18000;
    TIM2->ARR = 7000;
    TIM2->CR1 |= TIM_CR1_CKD_1;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM2_IRQn);

    //
    // Init and start both threads. We do not check status since all the params
    // guaranteed to be valid.
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
        &thread_idle, 
        idle_thread_fn, 
        NULL, 
        30, 
        thread_idle_stk, 
        sizeof(thread_idle_stk), 
        false
    ); 
}

//
// Exception handler just stop the system.
//
void system_stop(void)
{ 
    while(1)
        ; 
}

//
// Application entry point.
//
int main(void)
{
    SystemInit();

    //
    // // Enable HSE and wait until it is ready.
    //
    RCC->CR |= RCC_CR_HSEON;            
    while(!(RCC->CR & RCC_CR_HSERDY))
        ;
    
    //
    // Configure latency (0b010 should be used if sysclk > 48Mhz).
    //
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;

    //
    // Switch to HSE, configure PLL multiplier and set HSE as PLL source.
    //
    RCC->CFGR |= RCC_CFGR_SW_HSE;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;
    RCC->CFGR |= RCC_CFGR_PLLSRC;

    //
    // Enable PLL and wait until it is ready.
    //
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY))
        ;
    
    //
    // Set PLL as clock source and wait until it switches.
    //
    RCC->CFGR = (RCC->CFGR | RCC_CFGR_SW_PLL) & ~RCC_CFGR_SW_HSE;
    while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));

    //
    // The CPU is now running at 72MHz frequency.
    // It is safe to disable HSI.
    //
    RCC->CR &= ~RCC_CR_HSION;

    //
    // Setup PendSV and SysTick prioritiy at lowest possible level.
    // It is required for proper kernel initialization.
    //
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
    NVIC_SetPriority(SysTick_IRQn, 0xFF);
    NVIC_SetPriorityGrouping(3);

    //
    // Start system tick source.
    //
    SysTick->LOAD  = 72000 - 1;
    SysTick->VAL   = 0;
    SysTick->CTRL  = (1 << 2) | (1 << 1) | (1 << 0);

    //
    // Start scheduling loop.
    //
    fx_kernel_entry();
    return 0;
}

