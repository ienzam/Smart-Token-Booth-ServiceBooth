#This makefile is used to generate the hex file by avr gcc for atmega8
#and deploy it by usbasp device

CC = avr-gcc
GENHEX = avr-objcopy
MCFLAG = -mmcu=$(MCNAME)
CFLAGS = $(MCFLAG) -Os -c
HEXFLAGS = -j .text -j .data -O ihex

OBJECTS = lcd.o booth.o
ELFNAME = booth.elf
HEXNAME = booth.hex
#change MCNAME to needed architecture
MCNAME = atmega8

#SUBSYSTEM=="usb", SYSFS{idProduct}=="05dc", SYSFS{idVendor}=="16c0", GROUP="adm", MODE="0666"
#for deploy
AVRDUDE = avrdude
#change PROGRAMMER to appropriate programmer you are using to write to uc
PROGRAMMER = usbasp
#change ADMCNAME to appropriate architecture
ADMCNAME = m8

all: $(HEXNAME)

$(HEXNAME): $(ELFNAME)
	$(GENHEX) $(HEXFLAGS) $(ELFNAME) $(HEXNAME)

$(ELFNAME): $(OBJECTS)
	$(CC) $(MCFLAG) $(OBJECTS) -o $@

booth.o: servicebooth.c config.h
	$(CC) $(CFLAGS) $< -o $@

lcd.o: lcd.c lcd.h
	$(CC) $(CFLAGS) $< -o $@

deploy: $(ONAME).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(ADMCNAME) -U flash:w:$(ONAME).hex:i -v

clean:
	rm -f $(OBJECTS) $(ELFNAME) $(HEXNAME)
