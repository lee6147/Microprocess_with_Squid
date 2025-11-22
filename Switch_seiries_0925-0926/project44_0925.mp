Project:      ..\project44_0925\project44_0925
Build Number: 2
Date:         Thu Sep 25 15:12:47 2025
Compiler:     ICCAVR
Version:      8.04.03

NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                           idata   008C   0002 =      2. bytes (rel,con,rom,lit)

       Addr  Global Symbol
      -----  --------------------------------
       008C  __idata_start
       008E  __idata_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   008E   00A8 =    168. bytes (rel,con,rom,code)

       Addr  Global Symbol
      -----  --------------------------------
       0047  __start
       0047  __text_start
       0069  _exit
       006A  _main
       007C  _delay_m
       0091  push_xgsetF000
       0096  pop_xgsetF000
       009B  __text_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            data   0100   0002 =      2. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       0100  _LowHigh
       0100  __data_start
       0102  __data_end

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

