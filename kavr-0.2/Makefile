##############################################################################
#
#   kavr
#
#   AVR Hexfile Boot Loader (pronounced "caviar")
#
#   Copyright (C) 2009 Frank Edelhaeuser
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
##############################################################################

#
#   Boot loader customization
#   =========================
#
#   MCU
#
#       AVR variant (atmega8, attiny85...). For a list of known MCU names, 
#       see the output of "avr-gcc --target-help".
#
#   F_CPU
#
#       CPU clock, in Hertz (depends on hardware clock source)
#
#   BOOTADDR
#
#       Specifies the start address of the boot loader in flash, in bytes. 
#       See AVR datasheet for a list of valid addresses. Please note that 
#       the datasheet usually specifies word addresses (e.g. word address 
#       0xF00 is byte address 0x1E00).
#
#       Make sure the compiled boot loader fits into the flash address
#       space above $(BOOTADDR). Inspect the boot loader hex file for the
#       largest used flash address.
#
#   FUSE_L
#   FUSE_H
#
#       Fuse low and high bytes. See AVR datasheet. Fuse settings shall 
#       match with the other settings. In particular:
#
#           - CLKSEL and SUT should match the hardware clock
#             circuit and should result in the CPU clock specified 
#             with $(F_CPU.)
#
#           - BOOTSZ should match the boot loader start address 
#             specified with $(BOOTADDR).
#
#           - BOOTRST should be set to 0.
#
#   BAUD
#
#       Specifies the baud rate to use for serial communication. Make 
#       sure your clock configuration supports this baud rate without\
#       significant error.
#
#   TIMEOUT
#
#       Specifies how long in milliseconds the boot loader is supposed
#       to wait for valid data records until launching the application
#
#   DUDEPORT
#
#       Specifies avrdude command line parameters for configuring your 
#       programmer. At least your programmer type (-c <programmer>), 
#       and any port or timing parameters it may require should be 
#       provided. The makefile automatically appends the CPU model 
#       from $(MCU).
#
#   TESTPORT, TESTAPP
#
#       Specifies the serial port and application file to be used with 
#       the built-in test targets. Downloading an application using 
#       KAVR can be tested using "make test_program". This simple test 
#       will use DOS command line tools mode and print. Calling
#       "make test_verify" will compare the application in flash with
#       the hexfile.
#
MCU       = atmega8
F_CPU     = 8000000
BAUD      = 38400
TIMEOUT   = 1000

# 512-byte bootloader (see comments above)
BOOTADDR  = 0x1e00
# 16MHz
#FUSE_L    = 0x20
#FUSE_H    = 0xdc
# No BL
FUSE_L    = 0x20
FUSE_H    = 0xdd
# 8MHz
#FUSE_L    = 0x24
#FUSE_H    = 0xdc

# Your programmer, your serial port, your main app...
#DUDEPORT  = -c dragon_isp -P usb
DUDEPORT  = -c dragon_isp -P usb
TESTPORT  = com5:
TESTAPP   = testapp.hex


#
# Anything below should NOT require modification
# ==============================================
#
PROJECT  = kavr
LFLAGS   = -Wl,-Ttext,$(BOOTADDR) -nodefaultlibs -nostartfiles
CFLAGS   = -g -Os -Wall -mmcu=$(MCU) -I. -DF_CPU=$(F_CPU) -DTIMEOUT=$(TIMEOUT) -DBAUD=$(BAUD) -DBOOTADDR=$(BOOTADDR)
AVRDUDE  = avrdude $(DUDEPORT) -p $(MCU) -y -u 
OBJS     = kavr.o


all: kavr.hex

clean:
	rm -f *.hex *.lst *.map *.elf *.o

fuse:
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m -B20

flash: kavr.hex
	$(AVRDUDE) -U flash:w:$^:i

.c.o:
	avr-gcc -c $(CFLAGS) $< -o $@

%.elf: $(OBJS)
	avr-gcc $(LFLAGS) -Wl,-Map,$@.map -o $@ $^
	avr-objdump --source -d $@ > $@.lst
	avr-size --common -d $@

%.hex : %.elf
	avr-objcopy -j .text -O ihex $^ $@

# Simple test facility
test_dude:
	$(AVRDUDE) -t

test_program:
	mode $(TESTPORT) BAUD=$(BAUD) DATA=8 PARITY=N STOP=1 XON=on TO=off
	print /d:$(TESTPORT) $(TESTAPP)

test_verify:
	$(AVRDUDE) -U flash:v:$(TESTAPP):i
