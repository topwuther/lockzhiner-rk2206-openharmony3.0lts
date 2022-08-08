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

#include "link.h"

VENDOR_LOAD_ADDR        = XIP_ADDR + FIRMWARE_VENDOR_OFFSET;
VENDOR_LOAD_SIZE	= FIRMWARE_WIFI_OFFSET - FIRMWARE_VENDOR_OFFSET;

WIFI_LOAD_ADDR		= VENDOR_LOAD_ADDR + VENDOR_LOAD_SIZE;
WIFI_LOAD_SIZE		= FIRMWARE_WIFI_SIZE;

XIP_LOAD_ADDR		= WIFI_LOAD_ADDR + WIFI_LOAD_SIZE;
XIP_LOAD_SIZE		= PART_LITEOS_BLOCKS * PART_BLOCK_SIZE - (XIP_LOAD_ADDR - XIP_ADDR);

/* Entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
  SRAM_CODE (rwx)	: ORIGIN = SRAM_CODE_ADDR, LENGTH = SRAM_CODE_SIZE
  SRAM_DATA (rwx)	: ORIGIN = SRAM_DATA_ADDR, LENGTH = SRAM_DATA_SIZE
  PSRAM (rwx)           : ORIGIN = PSRAM_ADDR, LENGTH = PSRAM_SIZE
  VENDOR (rx)           : ORIGIN = VENDOR_LOAD_ADDR, LENGTH = VENDOR_LOAD_SIZE
  WIFI (rx)        	: ORIGIN = WIFI_LOAD_ADDR, LENGTH = WIFI_LOAD_SIZE
  XIP (rx)        	: ORIGIN = XIP_LOAD_ADDR, LENGTH = XIP_LOAD_SIZE
}

SECTIONS
{
  .isr.vector :
  {
    . = ALIGN(512);
    KEEP(*(.isr_vector))               /* vector table */
    . = ALIGN(512);
    *startup_rk2206.o (.text .text.*)
    . = ALIGN(4);
  } >VENDOR

  .wifi_firmwave :
  {
    . = ALIGN(512);
    KEEP(*(.wifi_firmwave))               /* wifi firmware */
    . = ALIGN(512);
    . = ALIGN(4);
  } >WIFI

  /* zInit code and data - will be freed after init */
  .zInit :
  {
     __zinitcall_bsp_start = .;
     KEEP (*(.zinitcall.bsp0.init))
     KEEP (*(.zinitcall.bsp1.init))
     KEEP (*(.zinitcall.bsp2.init))
     KEEP (*(.zinitcall.bsp3.init))
     KEEP (*(.zinitcall.bsp4.init))
     __zinitcall_bsp_end = .;
     __zinitcall_device_start = .;
     KEEP (*(.zinitcall.device0.init))
     KEEP (*(.zinitcall.device1.init))
     KEEP (*(.zinitcall.device2.init))
     KEEP (*(.zinitcall.device3.init))
     KEEP (*(.zinitcall.device4.init))
     __zinitcall_device_end = .;
     __zinitcall_core_start = .;
     KEEP (*(.zinitcall.core0.init))
     KEEP (*(.zinitcall.core1.init))
     KEEP (*(.zinitcall.core2.init))
     KEEP (*(.zinitcall.core3.init))
     KEEP (*(.zinitcall.core4.init))
     __zinitcall_core_end = .;
     __zinitcall_sys_service_start = .;
     KEEP (*(.zinitcall.sys.service0.init))
     KEEP (*(.zinitcall.sys.service1.init))
     KEEP (*(.zinitcall.sys.service2.init))
     KEEP (*(.zinitcall.sys.service3.init))
     KEEP (*(.zinitcall.sys.service4.init))
     __zinitcall_sys_service_end = .;
     __zinitcall_sys_feature_start = .;
     KEEP (*(.zinitcall.sys.feature0.init))
     KEEP (*(.zinitcall.sys.feature1.init))
     KEEP (*(.zinitcall.sys.feature2.init))
     KEEP (*(.zinitcall.sys.feature3.init))
     KEEP (*(.zinitcall.sys.feature4.init))
     __zinitcall_sys_feature_end = .;
     __zinitcall_run_start = .;
     KEEP (*(.zinitcall.run0.init))
     KEEP (*(.zinitcall.run1.init))
     KEEP (*(.zinitcall.run2.init))
     KEEP (*(.zinitcall.run3.init))
     KEEP (*(.zinitcall.run4.init))
     __zinitcall_run_end = .;
     __zinitcall_app_service_start = .;
     KEEP (*(.zinitcall.app.service0.init))
     KEEP (*(.zinitcall.app.service1.init))
     KEEP (*(.zinitcall.app.service2.init))
     KEEP (*(.zinitcall.app.service3.init))
     KEEP (*(.zinitcall.app.service4.init))
     __zinitcall_app_service_end = .;
     __zinitcall_app_feature_start = .;
     KEEP (*(.zinitcall.app.feature0.init))
     KEEP (*(.zinitcall.app.feature1.init))
     KEEP (*(.zinitcall.app.feature2.init))
     KEEP (*(.zinitcall.app.feature3.init))
     KEEP (*(.zinitcall.app.feature4.init))
     __zinitcall_app_feature_end = .;
     __zinitcall_test_start = .;
     KEEP (*(.zinitcall.test0.init))
     KEEP (*(.zinitcall.test1.init))
     KEEP (*(.zinitcall.test2.init))
     KEEP (*(.zinitcall.test3.init))
     KEEP (*(.zinitcall.test4.init))
     __zinitcall_test_end = .;
     __zinitcall_exit_start = .;
     KEEP (*(.zinitcall.exit0.init))
     KEEP (*(.zinitcall.exit1.init))
     KEEP (*(.zinitcall.exit2.init))
     KEEP (*(.zinitcall.exit3.init))
     KEEP (*(.zinitcall.exit4.init))
     __zinitcall_exit_end = .;
  } >XIP

  _shdf_text = .;
  .hdf.text :
  {
    _hdf_text_start = .;
    . = ALIGN(4);
    *libhardware.a:*.o (.text .text.* .rodata .rodata*)
    *libboard.a:*.o (.text .text.* .rodata .rodata*)
    *libarch.a:*.o (.text .text.* .rodata .rodata*)
    *libkernel.a:*.o (.text .text.* .rodata .rodata*)
    *libgcc.a:*.o (.text .text.* .rodata .rodata*)
    *libc.a:*.o (.text .text.* .rodata .rodata*)
    *libcpup.a:*.o (.text .text.* .rodata .rodata*)
    . = ALIGN(4);
    _hdf_text_end = .;
  } >SRAM_CODE AT >XIP
 
  .hdf.driver :
  {
    _hdf_drivers_start = .;
    . = ALIGN(4);
    KEEP(*(.hdf.driver))
    . = ALIGN(4);
    _hdf_drivers_end = .;
  } >XIP
  /* The program code and other data goes into FLASH */
  .text :
  {
    _stext = .;
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >XIP
  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >XIP

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >XIP
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >XIP

  _sdata = .;
  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _data_start = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _data_end = .;        /* define a global symbol at data end */
  } >SRAM_DATA AT >XIP

  /* Uninitialized data section */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    . = ALIGN(4);
    _bss_start = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _bss_end = .;
  } >PSRAM

  .stack :
  {
    . = ALIGN(8);
    _sstack = .;
    PROVIDE ( end = . );
    *(.stack*)
    . = . + SYS_STACK_SIZE;
    _estack = .;
    PROVIDE(_StackTop = .);
    . = ALIGN(8);
  } >PSRAM

  .heap :
  {
    . = ALIGN(8);
    _heap_start = .;
    . = ALIGN(8);
  } >PSRAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
