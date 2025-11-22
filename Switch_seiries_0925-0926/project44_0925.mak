CFLAGS =  -e -D__ICC_VERSION=80403 -D__BUILD=0 -DATMega128  -l -A -g -MLongJump -MHasMul -MEnhanced -Wf-use_elpm 
ASFLAGS = $(CFLAGS) 
LFLAGS =  -g -nb:0 -e:0x20000 -ucrtatmega.o -bfunc_lit:0x8c.0x10000 -dram_end:0x10ff -bdata:0x100.0x10ff -dhwstk_size:20 -beeprom:0.4096 -fihx_coff -S2
FILES = main.o 

default:	$(FILES)
	$(CC) -o default $(LFLAGS) @..\project44_0925\project44_0925.lk  -lcatm128
