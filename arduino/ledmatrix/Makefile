TARGET_DEVICE ?= atmega328p
CPUFREQ ?= 16000000
PROGRAMMER ?= arduino
DEVICE_FILE ?= /dev/ttyACM0
BAUD_RATE ?= 115200

CPPFLAGS += -Wall -Wextra -pedantic -Werror
CXXFLAGS += -std=c++14
CPPFLAGS += -Os
CPPFLAGS += -g

.PHONY: all
all: ledmatrix.hex

ledmatrix: CC := avr-gcc
ledmatrix: CXX := avr-g++
ledmatrix: CPPFLAGS += -mmcu=$(TARGET_DEVICE) -DF_CPU=$(CPUFREQ)
ledmatrix: LDFLAGS += -mmcu=$(TARGET_DEVICE)
ledmatrix: CPPFLAGS += -mcall-prologues
ledmatrix: LDFLAGS += -mcall-prologues

ledmatrix.hex: ledmatrix
	avr-nm --size-sort $< | awk '{ s=strtonum("0x" $$1); sum[tolower($$2)]+=s; print s, $$2, $$3} END { for (s in sum) print sum[s], s, "total" }' | avr-c++filt
	avr-size $<
	avr-objcopy -O ihex -R .eeprom $< $@

.PHONY: clean
clean:
	rm -f ledmatrix *.hex

.PHONY: deploy
deploy: ledmatrix.hex
	avrdude -c $(PROGRAMMER) -p $(TARGET_DEVICE) -P $(DEVICE_FILE) -b $(BAUD_RATE) -U flash:w:$< -v
