VENDOR_LOAD_ADDR = 0x10000000 + (((0x00 + 0x80 * 0X200) + 0x80 * 0X200) + 1 * 0X200);;
VENDOR_LOAD_SIZE = 0x20c00 - (((0x00 + 0x80 * 0X200) + 0x80 * 0X200) + 1 * 0X200);;
WIFI_LOAD_ADDR = VENDOR_LOAD_ADDR + VENDOR_LOAD_SIZE;
WIFI_LOAD_SIZE = 0x40000;
XIP_LOAD_ADDR = WIFI_LOAD_ADDR + WIFI_LOAD_SIZE;
XIP_LOAD_SIZE = 0x0F00 * 0X200 - (XIP_LOAD_ADDR - 0x10000000);
ENTRY(Reset_Handler)
MEMORY
{
  SRAM_CODE (rwx) : ORIGIN = 0x00080000, LENGTH = 0x26000
  SRAM_DATA (rwx) : ORIGIN = (0x20000000 + 0x26000), LENGTH = (0x20000 + 0x20000 - 0x26000)
  PSRAM (rwx) : ORIGIN = 0x38000000, LENGTH = 0x00800000
  VENDOR (rx) : ORIGIN = VENDOR_LOAD_ADDR, LENGTH = VENDOR_LOAD_SIZE
  WIFI (rx) : ORIGIN = WIFI_LOAD_ADDR, LENGTH = WIFI_LOAD_SIZE
  XIP (rx) : ORIGIN = XIP_LOAD_ADDR, LENGTH = XIP_LOAD_SIZE
}
SECTIONS
{
  .isr.vector :
  {
    . = ALIGN(512);
    KEEP(*(.isr_vector))
    . = ALIGN(512);
    *startup_rk2206.o (.text .text.*)
    . = ALIGN(4);
  } >VENDOR
  .wifi_firmwave :
  {
    . = ALIGN(512);
    KEEP(*(.wifi_firmwave))
    . = ALIGN(512);
    . = ALIGN(4);
  } >WIFI
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
  .text :
  {
    _stext = .;
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >XIP
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >XIP
  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >XIP
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >XIP
  _sdata = .;
  .data :
  {
    . = ALIGN(4);
    _data_start = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _data_end = .;
  } >SRAM_DATA AT >XIP
  .bss :
  {
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
    . = . + 0x100000;
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
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
