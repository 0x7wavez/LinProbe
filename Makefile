CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99 -O2
CPPFLAGS ?= -Iinclude
LDFLAGS ?=

BINDIR ?= bin
SRCDIR ?= src

all: $(BINDIR)/elfscope $(BINDIR)/tracer

$(BINDIR)/elfscope: $(SRCDIR)/elfscope.c
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

$(BINDIR)/tracer: $(SRCDIR)/tracer.c
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(BINDIR)/elfscope $(BINDIR)/tracer

.PHONY: all clean
