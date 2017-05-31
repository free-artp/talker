# export PATH=/opt/rpi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin:$PATH

# /opt/rpi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf
MY_SYSROOT=../../rpi-sysroot

TARGET=talker
SOURCES=main.c comm.c config.c scheduler.c gates.c

CC=$(CROSS_COMPILE)-cc

NM=$(CROSS_COMPILE)-nm
LD=$(CROSS_COMPILE)-ld
CXX=$(CROSS_COMPILE)-g++
RANLIB=$(CROSS_COMPILE)-ranlib
AR=$(CROSS_COMPILE)-ar
CPP=$(CROSS_COMPILE)-cpp
OBJCOPY=$(CROSS_COMPILE)-objcopy



TOOLCHAIN := $(shell cd $(dir $(shell which $(CROSS_COMPILE)-gcc))/.. && pwd -P)
SYSROOT := $(TOOLCHAIN)/$(CROSS_COMPILE)/sysroot


#CCFLAGS = -O0 -g -Wall
CCFLAGS = -O0 -g -I$(MY_SYSROOT)/usr/include -DDEBUG -DINFO
#LDFLAGS = -L$(TOOLCHAIN)/arm-linux-gnueabihf/sysroot/lib/ -lpthread -lwiringPi
LDFLAGS = -L$(MY_SYSROOT)/lib/arm-linux-gnueabihf -L$(MY_SYSROOT)/usr/lib -lpthread -lwiringPi -liniparser


OBJECTS = $(SOURCES:%.c=%.o)

all : $(TARGET)

install: $(TARGET)
	@cp $(TARGET) /srv/scan/
	@cp $(TARGET).ini /srv/scan/

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean :
	@rm -f $(TARGET) $(OBJECTS)
.PHONY : clean

iniparser:	iniparser.o
	$(CC) iniparser.o -o iniparser $(LDFLAGS)
