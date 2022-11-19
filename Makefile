# Environment
MCB_TOOLS_PATH=/Applications/mcb32tools.app/Contents/Resources/Toolchain
TOOLS_PATH=$(MCB_TOOLS_PATH)/bin
TARGET=mipsel-mcb32-elf-
AR=$(TOOLS_PATH)/$(TARGET)ar
AS=$(TOOLS_PATH)/$(TARGET)as
LD=$(TOOLS_PATH)/$(TARGET)ld
CC=$(TOOLS_PATH)/$(TARGET)gcc
CXX=$(TOOLS_PATH)/$(TARGET)g++
CPP=$(TOOLS_PATH)/$(TARGET)cpp
BIN2HEX=$(TOOLS_PATH)/$(TARGET)bin2hex
AVRDUDE=$(TOOLS_PATH)/$(TARGET)avrdude
ASFLAGS=-march=mips32r2 -msoft-float
C_INCLUDE_PATH=-I$(MCB_TOOLS_PATH)/lib/gcc/mipsel-mcb32-elf/4.9.2/include -I$(MCB_TOOLS_PATH)/include
CFLAGS=-march=mips32r2 -msoft-float -Wa,-msoft-float -G 0 $(C_INCLUDE_PATH)
LDFLAGS=-L $(MCB_TOOLS_PATH)/lib/proc -L $(MCB_TOOLS_PATH)/lib

# PIC32 device number
DEVICE		= 32MX320F128H

# UART settings for programmer
TTYDEV		?=/dev/tty.usbserial-A503WFH1
TTYBAUD		?=115200

SRCDIR = src
OUTDIR = out

# Name of the project
PROGNAME	= $(OUTDIR)/outfile

# Linkscript
LINKSCRIPT	:= p$(shell echo "$(DEVICE)" | tr '[:upper:]' '[:lower:]').ld

# Compiler and linker flags
CFLAGS		+= -ffreestanding -march=mips32r2 -msoft-float -Wa,-msoft-float
ASFLAGS		+= -msoft-float
LDFLAGS		+= -T $(LINKSCRIPT)

# Filenames
ELFFILE		= $(PROGNAME).elf
HEXFILE		= $(PROGNAME).hex

# Find all source files automatically
CFILES          = $(wildcard $(SRCDIR)/*.c)
ASFILES         = $(wildcard $(SRCDIR)/*.S)
SYMSFILES				= $(wildcard $(SRCDIR)/*.syms)

# Object file names
OBJFILES        = $(CFILES:.c=.c.o)
OBJFILES        +=$(ASFILES:.S=.S.o)
OBJFILES				+=$(SYMSFILES:.syms=.syms.o)

# Hidden directory for dependency files
DEPDIR = .deps
df = $(DEPDIR)/$(*F)

.PHONY: all clean install
.SUFFIXES:

all: $(HEXFILE)

clean:
	$(RM) -R $(OUTDIR)
	$(RM) $(OBJFILES)
	$(RM) -R $(DEPDIR)

install:
	$(AVRDUDE) -v -p $(shell echo "$(DEVICE)" | tr '[:lower:]' '[:upper:]') -c stk500v2 -P "$(TTYDEV)" -b $(TTYBAUD) -U "flash:w:$(HEXFILE)"

$(OUTDIR):
	@mkdir -p $@

$(ELFFILE): $(OUTDIR) | $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $(OBJFILES) $(LDFLAGS)

$(HEXFILE): $(OUTDIR) | $(ELFFILE)
	$(BIN2HEX) -a $(ELFFILE)

$(DEPDIR):
	@mkdir -p $@

# Compile C files
%.c.o: %.c $(DEPDIR)
	$(CC) $(CFLAGS) -c -MD -o $@ $<
	@cp $*.c.d $(df).c.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.c.d >> $(df).c.P; $(RM) $*.c.d

# Compile ASM files with C pre-processor directives
%.S.o: %.S $(DEPDIR)
	$(CC) $(CFLAGS) $(ASFLAGS) -c -MD -o $@ $<
	@cp $*.S.d $(df).S.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.S.d >> $(df).S.P; $(RM) $*.S.d

# Link symbol lists to object files
%.syms.o: %.syms
	$(LD) -o $@ -r --just-symbols=$<

# Check dependencies
-include $(CFILES:%.c=$(DEPDIR)/%.c.P)
-include $(ASFILES:%.S=$(DEPDIR)/%.S.P)
