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
 */

.syntax unified
.cpu cortex-m3
.thumb

.extern main
.extern hal_swi_entry
.extern hal_tick_entry
.extern hal_intr_entry
.extern system_stop

.section .vectors, "ax"
.code 16
.align 0
.global vectors

vectors:
.word     0x20005000                  // Top of stack
.word     RESET_Handler       	      // Reset Handler
.word     system_stop                 // NMI
.word     system_stop                 // HardFault
.word     system_stop                 // MPU Fault Handler
.word     system_stop                 // Bus Fault Handler
.word     system_stop                 // Usage Fault Handler
.word     0                           // Reserved
.word     0                           // Reserved
.word     0                           // Reserved
.word     0                           // Reserved
.word     system_stop                 // SVCall Handler
.word     system_stop                 // Debug Monitor Handler
.word     0                           // Reserved
.word     hal_swi_entry               // PendSV Handler
.word     hal_tick_entry              // SysTick Handler
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry
.word     hal_intr_entry

.section .text
.global RESET_Handler
.thumb_func
RESET_Handler:

init_bss_start:
  ldr   r0, =0
  ldr   r1, =_bss
  ldr   r2, =_bss_sz 
  cmp   r2, $0
  beq   image_init_done
init_bss:
  strb  r0, [r1]
  adds  r1, r1, $1
  subs  r2, r2, $1
  bne   init_bss
        
image_init_done:
  cpsid i
  b main                              // Jump to C entry.
