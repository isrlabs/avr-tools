#############
# TOOLCHAIN #
#############

CC = avr-gcc
LD = avr-ld
STRIP = avr-strip
OBJCOPY = avr-objcopy
SOURCES = serial.c
SIZE = avr-size


##############
# PARAMETERS #
##############

TARGET = ${.CURDIR:C/.*\///g}
PROGRAMMER = avrisp2
MCU = atmega328
PART = m328p
F_CPU = 16000000
PORT = usb
BINFORMAT = ihex
PGMPROTO = avrisp2
CFLAGS = -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -I.
AVRDUDE = avrdude -v -p $(PART) -c $(PROGRAMMER) -P $(PORT)

all: $(TARGET).hex

.elf.hex:

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O $(BINFORMAT) -R .eeprom $(TARGET).elf $(TARGET).hex
	$(SIZE) $(TARGET).hex

$(TARGET).elf: $(TARGET).c $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET).elf $(SOURCES) $(TARGET).c 
	$(STRIP) $(TARGET).elf

upload: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$(TARGET).hex

clean:
	rm -f *.hex *.elf *.eeprom

.PHONY: clean upload
