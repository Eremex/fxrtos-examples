Thread notification using semaphore
===================================

Demo description
----------------

The demo creates two threads printing its ids via UART console. First thread is
activated by system timer. Second thread is activated by the first thread using
semaphore.
The application demonstrates how to use threads and semaphores for notification.

Note: Timer tick is about 100ms length, so, 1 sec delay may be achieved by
using 10 ticks timeouts.

Note: There's no need for explicit synchronization between two threads. Since
they have equal priority, preemptions cannot occur.

Files listing
-------------

| File name       | Description
|-----------------|---------------------------------------------------------
| build.bat       | build script
| FXRTOS.h        | FX-RTOS interface header
| fxrtos_demo.elf | application binary
| libfxrtos.a     | FX-RTOS kernel
| SiFive.ld       | linker script and memory layout
| main.c          | privileged application and PFC handlers
| startup.S       | vector table

Building
--------

Prerequisites: 
compiler and linker must be directly accessible (via PATH environment variable).

1. Set GCC_PREFIX variable for toolchain being used 
   i.e. set GCC_PREFIX=riscv-none-embed-
2. Run build.bat

ELF-file containing application image should appear in demo folder.

Downloading to target
---------------------

Compiled image should be downloaded to the device by external tools.
Please, see documentation for your evaluation board.
