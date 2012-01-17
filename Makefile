# Makefile for macosx
#

CC ?= gcc
CFLAGS ?= -O2 -g -Wall -W -fPIC -I. -I..
#CFLAGS ?= -g3 -O0 -Wall -fPIC -I. -I..

logpool=logpool
version = 1.0
dir    = build
objs = \
	$(dir)/logpool.o\
	$(dir)/file.o\
	$(dir)/syslog.o\
	$(dir)/string.o

test = logtest

.PHONY: all
all: $(dir)/$(test)

$(dir)/$(test) : logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ logtest.c -L./$(dir) -l$(logpool)

$(dir)/lib$(logpool).dylib : $(objs)
	$(CC) $(CFLAGS) -dynamiclib $(LIBVER) -o $@ $^ $(LDLIBS)

## object files

$(dir)/logpool.o : logpool.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(dir)/file.o : ./plugins/file.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(dir)/syslog.o : ./plugins/syslog.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(dir)/string.o : ./plugins/string.c
	$(CC) $(CFLAGS) -c $^ -o $@

## clean
.PHONY: clean
clean:
	$(RM) -rf $(dir)/*.dylib $(dir)/*.o $(dir)/$(test)

