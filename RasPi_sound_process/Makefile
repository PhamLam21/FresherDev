TOOLCHAIN := /home/liam/PI/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
CC := $(TOOLCHAIN)gcc
CFLAGS := -Wall -O2 -I./inc -pthread
LDFLAGS = -L./lib -lAWECoreOS -lm -L./lib/External -lasound 

# Directories
SRCDIR := ./src
INCDIR := ./inc
BINDIR := ./bin

# Source and object files
SRC := $(wildcard $(SRCDIR)/*.c)
TARGET := $(BINDIR)/sound_process

all: $(BINDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS) -I./inc -I./inc/External/alsa
	$(CC) $(CFLAGS) -o $(BINDIR)/client client.c 
$(BINDIR):
	@mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR)

.PHONY: all clean