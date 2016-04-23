.PHONY: all
all: blink.elf

.SUFFIXES: .elf .c

CPPFLAGS += -mmcu=msp430afe253
CPPFLAGS += -std=c99
CC := msp430-gcc

.c.elf:
	$(CC) $(CPPFLAGS) $(OUTPUT_OPTION) $<

.PHONY: clean
clean:
	rm *.elf

.PHONY: upload
upload: blink.elf
	mspdebug -d /dev/ttyUSB0 uif "prog $<"
