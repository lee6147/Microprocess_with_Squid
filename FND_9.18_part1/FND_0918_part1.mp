Project:      ..\FND_0918_part1\FND_0918_part1
Build Number: 15
Date:         Sat Sep 20 20:42:15 2025
Compiler:     ICCAVR
Version:      8.04.03

NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                           idata   008C   000E =     14. bytes (rel,con,rom,lit)

       Addr  Global Symbol
      -----  --------------------------------
       008C  __idata_start
       009A  __idata_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   009A   0118 =    280. bytes (rel,con,rom,code)

       Addr  Global Symbol
      -----  --------------------------------
       004D  __start
       004D  __text_start
       006F  _exit
       0070  _main
       00BA  _delay_m
       00CF  push_xgsetF000
       00D4  pop_xgsetF000
       00D9  __text_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            data   0100   000E =     14. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       0100  _number
       0100  __data_start
       010A  _LOW_HIGH
       010C  _Ncount
       010D  _cnt
       010E  __data_end

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

