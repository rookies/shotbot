SKETCH    := arduino-software.ino
LIBRARIES := AccelStepper Servo
BAUDRATE  := 86400
BOARD     ?= arduino:avr:nano:cpu=atmega328old
ARDUINO   ?= arduino-cli
PICOCOM   ?= picocom
PORT      ?= /dev/ttyUSB0
LIB_DIR   ?= ~/Arduino/libraries

SRC    := $(SKETCH) \
          $(wildcard *.hh) \
          $(foreach lib,$(LIBRARIES),$(LIB_DIR)/$(lib) $(wildcard $(LIB_DIR)/$(lib)/*))
TARGET := ..$(subst $(noop) $(noop),.,$(filter-out cpu=%,$(subst :, ,$(BOARD))))

upload : $(TARGET).hex
	$(ARDUINO) upload -p $(PORT) --fqbn $(BOARD) .

compile : $(TARGET).hex

console :
	@echo "NOTE: Press Ctrl+A Ctrl+X to exit."
	$(PICOCOM) -b $(BAUDRATE) $(PORT)

$(TARGET).hex : $(SRC)
	$(ARDUINO) compile --fqbn $(BOARD) .

.PHONY : clean
clean :
	$(RM) $(TARGET).hex $(TARGET).elf
