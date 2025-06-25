CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
INCLUDES = -Iinclude
LDFLAGS = -lm

SRCDIR = src
BINDIR = bin
TARGET = main

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Default target
all: $(BINDIR)/$(TARGET)

# Create bin directory and compile
$(BINDIR)/$(TARGET): $(SOURCES)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $@ $(LDFLAGS)

# Clean
clean:
	rm -rf $(BINDIR)

.PHONY: all clean
