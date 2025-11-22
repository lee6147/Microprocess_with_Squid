Project:      ..\LED_Blink_retry_9.13\LED_Blink_retry_9.13
Build Number: 19
Date:         Sat Sep 13 16:38:44 2025
Compiler:     ICCAVR
Version:      8.04.03

NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   008C   00A2 =    162. bytes (rel,con,rom,code)

       Addr  Global Symbol
      -----  --------------------------------
       0046  __start
       0046  __text_start
       0068  _exit
       0069  _main
       0078  _delay_m
       008D  push_xgsetF000
       0092  pop_xgsetF000
       0097  __text_end

Area                               Start  End    Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                          vector   0000   0004 =      4. bytes (abs,ovr,rom,code)

Files Linked      [ module(s) ]

C:\iccv8avr\lib\crtatmega.o	[ crtatmega.s ]
.objs\main.o	[ main.c ]
<library>	[ gpushy.s ]

User Global Definitions

ram_end = 0x10ff
hwstk_size = 0x14

Area Definitions

-bfunc_lit:0x8c.0x10000
-bdata:0x100.0x10ff

