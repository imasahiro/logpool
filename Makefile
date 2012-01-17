# Makefile for macosx
#

CC ?= gcc
#CFLAGS ?= -O2 -g -Wall -W -fPIC -I. -I..
CFLAGS ?= -g3 -O0 -Wall -fPIC -I. -I..
LDLIBS = -lmemcached

logpool=logpool
version = 1.0
dir    = build
objs = \
	$(dir)/logpool.o\
	$(dir)/file.o\
	$(dir)/string.o\
	$(dir)/syslog.o\
	$(dir)/memcache.o\

.PHONY: all
all: $(dir)/test_string $(dir)/test_memcache $(dir)/test_syslog $(dir)/bench

$(dir)/test_string : test/logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool) -DLOGAPI_TEST -DLOGTEST_STRING_API

$(dir)/test_memcache : test/logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool) -DLOGAPI_TEST -DLOGTEST_MEMCACHE_API

$(dir)/test_syslog : test/logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool) -DLOGAPI_TEST -DLOGTEST_SYSLOG_API

$(dir)/bench : test/bench.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool)

$(dir)/lib$(logpool).dylib : $(objs)
	$(CC) $(CFLAGS) -dynamiclib $(LIBVER) -o $@ $^ $(LDLIBS)

## object files

$(dir)/logpool.o : logpool.c logpool.h
	$(CC) $(CFLAGS) -c $< -o $@

$(dir)/file.o : ./plugins/file.c logpool.h plugins/logpool_string.h
	$(CC) $(CFLAGS) -c $< -o $@

$(dir)/syslog.o : ./plugins/syslog.c logpool.h plugins/logpool_string.h
	$(CC) $(CFLAGS) -c $< -o $@

$(dir)/string.o : ./plugins/string.c logpool.h plugins/logpool_string.h
	$(CC) $(CFLAGS) -c $< -o $@

$(dir)/memcache.o : ./plugins/memcache.c logpool.h plugins/logpool_string.h
	$(CC) $(CFLAGS) -c $< -o $@

## clean
.PHONY: clean
clean:
	$(RM) -rf $(dir)/*

