/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* SPDX-License-Identifier: BSD-3-Clause */


/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/

  .syntax  unified
  .arch    armv7e-m

  .extern  _shdf_text
  .extern  _hdf_text_start
  .extern  _hdf_text_end

  .extern  _sdata
  .extern  _data_start
  .extern  _data_end

  .extern _bss_start
  .extern _bss_end

  .extern  IntDefaultHandler
  .extern  SystemInit
  .extern  Main

  .section .isr_vector
  .align   2
  .globl   __Vectors
  .globl   __Vectors_End
  .globl   __Vectors_Size
__Vectors:
  .long    _StackTop                         /*     Top of Stack */
  .long    Reset_Handler                      /*     Reset Handler */
  .long    NMI_Handler                        /* -14 NMI Handler */
  .long    HardFault_Handler                  /* -13 Hard Fault Handler */
  .long    MemManage_Handler                  /* -12 MPU Fault Handler */
  .long    BusFault_Handler                   /* -11 Bus Fault Handler */
  .long    UsageFault_Handler                 /* -10 Usage Fault Handler */
  .long    0                                  /*     Reserved */
  .long    0                                  /*     Reserved */
  .long    0                                  /*     Reserved */
  .long    0                                  /*     Reserved */
  .long    SVC_Handler                        /*  -5 SVCall Handler */
  .long    DebugMon_Handler                   /*  -4 Debug Monitor Handler */
  .long    0                                  /*     Reserved */
  .long    PendSV_Handler                     /*  -2 PendSV Handler */
  .long    SysTick_Handler                    /*  -1 SysTick Handler */

  /* Interrupts */
  .space   (64 * 4)                          /* Interrupts 0 .. 63 are left out */
__Vectors_End:
  .equ     __Vectors_Size, __Vectors_End - __Vectors
  .size    __Vectors, . - __Vectors

  .thumb
  .section .text
  .align   2

  .thumb_func
  .type    Reset_Handler, %function
  .globl   Reset_Handler
  .fnstart
Reset_Handler:
  ldr   sp, =_StackTop      /* set stack pointer */

/* Single HDF_TEXT section scheme.
 *
 * All addresses must be aligned to 4 bytes boundary.
 */
  ldr      r1, =_shdf_text
  ldr      r2, =_hdf_text_start
  ldr      r3, =_hdf_text_end

.L_loop_hdf:
  cmp      r2, r3
  ittt     lt
  ldrlt    r0, [r1], #4
  strlt    r0, [r2], #4
  blt      .L_loop_hdf

/* Single DATA section scheme.
 *
 * All addresses must be aligned to 4 bytes boundary.
 */
  ldr      r1, =_sdata
  ldr      r2, =_data_start
  ldr      r3, =_data_end

.L_loop_data:
  cmp      r2, r3
  ittt     lt
  ldrlt    r0, [r1], #4
  strlt    r0, [r2], #4
  blt      .L_loop_data

/* Single BSS section scheme.
 *
 * Both addresses must be aligned to 4 bytes boundary.
 */
  ldr      r1, =_bss_start
  ldr      r2, =_bss_end

  movs     r0, 0
.L_loop_bss:
  cmp      r1, r2
  itt      lt
  strlt    r0, [r1], #4
  blt      .L_loop_bss

  bl       SystemInit
  bl       Main

  .fnend
  .size    Reset_Handler, . - Reset_Handler


  .thumb_func
  .type    Default_Handler, %function
  .weak    Default_Handler
  .fnstart
Default_Handler:
  b        .
  .fnend
  .size    Default_Handler, . - Default_Handler

/* Macro to define default exception/interrupt handlers.
 * Default handler are weak symbols with an endless loop.
 * They can be overwritten by real handlers.
 */
  .macro   Set_Default_Handler  Handler_Name
  .weak    \Handler_Name
  .set     \Handler_Name, Default_Handler
  .endm

/* Default exception/interrupt handler */

   Set_Default_Handler  NMI_Handler
   Set_Default_Handler  HardFault_Handler
   Set_Default_Handler  MemManage_Handler
   Set_Default_Handler  BusFault_Handler
   Set_Default_Handler  UsageFault_Handler
   Set_Default_Handler  SVC_Handler
   Set_Default_Handler  DebugMon_Handler
   Set_Default_Handler  PendSV_Handler
   Set_Default_Handler  SysTick_Handler

   .end
