# Makefile for macosx
#

CC ?= gcc
CFLAGS ?= -O2 -g -Wall -W -fPIC -I. -I..
#CFLAGS ?= -g3 -O0 -Wall -fPIC -I. -I..
LDLIBS = -lmemcached

logpool=logpool
version = 1.0
dir    = build
objs = \
	$(dir)/logpool.o\
	$(dir)/file2.o\
	$(dir)/string.o\
	$(dir)/syslog.o\
	$(dir)/memcache.o\

tests = \
	$(dir)/test_string\
	$(dir)/test_memcache\
	$(dir)/test_syslog\
	$(dir)/test_file\
	$(dir)/bench

.PHONY: all
all: $(tests)

$(dir)/test_string : test/logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool) -DLOGAPI_TEST -DLOGTEST_STRING_API

$(dir)/test_memcache : test/logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool) -DLOGAPI_TEST -DLOGTEST_MEMCACHE_API

$(dir)/test_syslog : test/logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool) -DLOGAPI_TEST -DLOGTEST_SYSLOG_API

$(dir)/test_file : test/logtest.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/logtest.c -L./$(dir) -l$(logpool) -DLOGAPI_TEST -DLOGTEST_FILE_API

$(dir)/bench : test/bench.c $(dir)/lib$(logpool).dylib
	$(CC) $(CFLAGS) -o $@ test/bench.c -L./$(dir) -l$(logpool)

$(dir)/lib$(logpool).dylib : $(objs)
	$(CC) $(CFLAGS) -dynamiclib $(LIBVER) -o $@ $^ $(LDLIBS)

## object files

$(dir)/logpool.o : logpool.c logpool.h
	$(CC) $(CFLAGS) -c $< -o $@

$(dir)/file2.o : ./plugins/file2.c logpool.h plugins/logpool_string.h
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

