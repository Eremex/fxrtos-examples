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
 */

  .section .reset
  .section .vectors, "ax"

  .globl vector_base
vector_base:
  j reset_handler

  .globl reset_handler
  .type reset_handler, @function

reset_handler:
  /* Jump to real flash address at 0x0800 */
  la   a0, _start
  li   a1, 1
  slli a1, a1, 27
  bleu a1, a0, _start
  add  a0, a0, a1
  jr   a0

  .section .init, "ax", @progbits
  .global _start
  .type _start, @function
_start:
  /* load mtvec */
  la a0, hal_intr_entry
  csrw mtvec, a0
  csrc    mstatus, 8
  /* enable M mode time interrupts */
  li      t0, 0x80
  csrw    mie, t0

  /* Setup stack */
  la sp, main_stack
  addi    sp, sp, 0x200

  /* Load data section */
  la a0, _data_loadaddr
  la a1, _data
  la a2, _edata
  bgeu a1, a2, data_skip
data_init:
  lw t0, (a0)
  sw t0, (a1)
  addi a0, a0, 4
  addi a1, a1, 4
  bltu a1, a2, data_init
data_skip:

  /* Zero bss section */
  la a0, _bss
  la a1, _ebss
  bgeu a0, a1, bss_skip
bss_init:
  sw zero, (a0)
  addi a0, a0, 4
  bltu a0, a1, bss_init
bss_skip:

  /* Jump to main() */
  li a0, 0
  li a1, 0
  la t1, main
  jalr t1

.global mtimer_int_enable
.global mtimer_int_disable

mtimer_int_enable:
    csrr    a0, mie
    or      a0, a0, 0x80
    csrw    mie, a0
    ret

mtimer_int_disable:
    csrr    a0, mie
    li      a1, 0xffffff7f
    and     a0, a0, a1
    csrw    mie, a0
    ret
