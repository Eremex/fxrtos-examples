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
 *  The demo creates three LED-blinking threads.
 */

#include <MDR32F9Qx_rst_clk.h>
#include <FXRTOS.h>

//
// This demo does not handle interrupts.
//
void fx_intr_handler(void)
{
    ;
}

//
// LED-blinking thread 1.
//
static void thread_1(void* arg)
{
    while(1)
    {
        MDR_PORTB->RXTX |= 1;
        fx_thread_sleep(125);
        MDR_PORTB->RXTX &= ~1;
        fx_thread_sleep(125);
    }
}

//
// LED-blinking thread 2.
//
static void thread_2(void* arg)
{
    while(1)
    {
        MDR_PORTB->RXTX |= 2;
        fx_thread_sleep(250);
        MDR_PORTB->RXTX &= ~2;
        fx_thread_sleep(250);
    }
}

//
// LED-blinking thread 3.
//
static void thread_3(void* arg)
{
    while(1)
    {
        MDR_PORTB->RXTX |= 4;
        fx_thread_sleep(500);
        MDR_PORTB->RXTX &= ~4;
        fx_thread_sleep(500);
    }
}

//
// Application definition.
//
void fx_app_init(void)
{
    static fx_thread_t t1, t2, t3; 
    
    static int stack0[ 0x80 ];
    static int stack1[ 0x80 ];
    static int stack2[ 0x80 ];

    fx_thread_init(&t1, thread_1, NULL, 10, stack0, sizeof(stack0), false);
    fx_thread_init(&t2, thread_2, NULL, 11, stack1, sizeof(stack1), false);
    fx_thread_init(&t3, thread_3, NULL, 12, stack2, sizeof(stack2), false);
}

//
// Exception handler just stop the system.
//
void system_stop(void)
{
    while (1); 
}

//
// Set CPU frequency to 80 Mhz and enable GPIOB peripheral clock
//
void static
clk_config(void)
{
	RST_CLK_DeInit();

	RST_CLK_HSEconfig(RST_CLK_HSE_ON);
	if (RST_CLK_HSEstatus() == ERROR)
    {
        for(;;);
    }

	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10);

	RST_CLK_CPU_PLLcmd(ENABLE);
	if (RST_CLK_CPU_PLLstatus() == ERROR)
    {
        for (;;);
    }

	RST_CLK_CPU_PLLuse(ENABLE);
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);

    RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB, ENABLE);
}

//
// Application entry point.
//
int main(void)
{
    clk_config();

    //
    // Configure GPIOs for led driving.
    //
    MDR_PORTB->OE     = 7;
    MDR_PORTB->FUNC   = 0x00000000;
    MDR_PORTB->ANALOG = 0x0000FFFF;
    MDR_PORTB->PULL   = 7 << 16;
    MDR_PORTB->PD     = ~7;
    MDR_PORTB->PWR    = 0x55555555;
    MDR_PORTB->GFEN   = 0;
    MDR_PORTB->RXTX   = 0xFFFF;

    //
    // Set the lowest possible priority for PendSV and systick interrupts.
    //
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
    NVIC_SetPriority(SysTick_IRQn, 0xFF);
    
    //
    // Start system tick source.
    //
    SysTick->LOAD  = 80000 - 1;
    SysTick->VAL   = 0;
    SysTick->CTRL  = (1 << 2) | (1 << 1) | (1 << 0) ;

    //
    // Kernel start. This function must be called with interrupts disabled.
    //
    fx_kernel_entry();
    return 0;
}
