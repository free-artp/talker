# export PATH=/opt/rpi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin:$PATH

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-


TARGET=talker
SOURCES=main.c comm.c crc.c

CC=$(CROSS_COMPILE)cc

NM=$(CROSS_COMPILE)nm
LD=$(CROSS_COMPILE)ld
CXX=$(CROSS_COMPILE)g++
RANLIB=$(CROSS_COMPILE)ranlib
AR=$(CROSS_COMPILE)ar
CPP=$(CROSS_COMPILE)cpp
OBJCOPY=$(CROSS_COMPILE)objcopy



TOOLCHAIN := $(shell cd $(dir $(shell which $(CROSS_COMPILE)gcc))/.. && pwd -P)

#CCFLAGS = -O0 -g -Wall
CCFLAGS = -O0 -g
LDFLAGS = -L$(TOOLCHAIN)/arm-linux-gnueabihf/sysroot/lib/ -l pthread


OBJECTS = $(SOURCES:%.c=%.o)

all : $(TARGET)

install: $(TARGET)
	cp $(TARGET) /srv/scan/

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean :
	@rm -f $(TARGET) $(OBJECTS)
.PHONY : clean
