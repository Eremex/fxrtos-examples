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
 *  FX-RTOS demo application for RISC-V Board compatible with SiFive E SDK.
 *  Two threads printing its ids to UART.
 */

#include "FXRTOS.h"

int main_stack[0x200 / sizeof(int)];

static volatile uint32_t* const uart = (volatile uint32_t*) 0x10013000;
static volatile uint64_t* const mtimecmp = (volatile uint64_t*) 0x2004000;
static fx_sem_t sem;

extern void timer_enable(void);
extern void timer_disable(void);

static inline void timer_setup(void)
{
    uint64_t temp = *mtimecmp;
    temp += 1000000;
    *mtimecmp = temp;
}

/* UART interrupts are disabled after reset.
 * TXDATA register is mapped at UART base address. Register layout:
 * [31]      - TX FIFO full. 
 * [30 .. 8] - reserved
 * [7  .. 0] - symbol to transmit
 */

static inline void uart_tx(volatile uint32_t* txreg, char* str)
{
    while (*str)
    {
        while (*txreg & 0x80000000) /* Wait until FIFO has room for next sym. */
            ;
        
        *txreg = *str++;
    }
}

/*
 * Timer handling in the kernel is implemented as follows:
 * 1. interrupt_entry (intr disabled by by hardware)
 * 2. pre_tick
 * 3. interrupt_enable
 * 4. OS_tick
 * 5. interrupt_disable
 * 6. post_tick
 * 7. interrupt_leave (enabling interrupts globally)
 * pre/post ticks must be implemented as external functions to allow OS tick
 * handling with interrupts globally enabled. Since MTIMER always sets intr
 * pending whenever its compare value is equal to or less than the counter
 * we should disable timer interrupt before MIE flag is set in order to avoid
 * infinite ISR nesting. After OS processing the timer should be reprogrammed
 * to deassert interrupt and only after MTIP is reset interrupts may be again
 * globally enabled.
 */

void hal_timer_pre_tick(void)
{
    timer_disable();
}

void hal_timer_post_tick(void)
{
    timer_setup();
    timer_enable();
}

/* Two threads printing into UART. */

void thread1_fn(void* arg)
{
    while (1)
    {
        uart_tx(uart, "thread 1\r\n");
        fx_sem_post(&sem);
        fx_thread_sleep(10);
    }
}

void thread2_fn(void* arg)
{
    while (1)
    {
        fx_sem_timedwait(&sem, FX_THREAD_INFINITE_TIMEOUT);
        uart_tx(uart, "thread 2\r\n");
    }
}

/* Application definition. Called by the kernel after initialization. */

void fx_app_init(void)
{
    static fx_thread_t t1;
    static fx_thread_t t2;
    static int t1_stk[0x100];
    static int t2_stk[0x100];

    uart_tx(uart, "FX-RTOS RISC-V demo\r\n");

    fx_sem_init(&sem, 0, 1, FX_SYNC_POLICY_FIFO);
    fx_thread_init(&t1, thread1_fn, NULL, 5, t1_stk, sizeof(t1_stk), false); 
    fx_thread_init(&t2, thread2_fn, NULL, 5, t2_stk, sizeof(t2_stk), false); 
}

/* Program entry point. Interrupts are disabled by startup code. */

int main(void)
{
    extern char _bss;
    extern char _ebss;
    char* bss_start = &_bss;
    char* bss_end = &_ebss;

    while (bss_start != bss_end)
    {
        *(bss_start++) = 0;
    }

    timer_setup();
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
