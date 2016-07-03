#MCU=at90usb162 # Target is a Minimus
MCU=atmega32u2 # Target is a Minimus 32
F_CPU=2000000
OBJECTS=main.o
TARGET=program
CFLAGS=-g -Os -Wall -mcall-prologues -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CC=avr-gcc

all : $(TARGET).hex

$(TARGET).elf : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

%.hex : %.elf
	avr-objcopy -R .eeprom -O ihex $< $@

flash : $(TARGET).hex
	dfu-programmer $(MCU) erase
	dfu-programmer $(MCU) flash $(TARGET).hex
	dfu-programmer $(MCU) start

clean :
	rm -f *.hex *.elf *.o
