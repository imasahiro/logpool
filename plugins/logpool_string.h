#ifndef LOGPOOL_STRING_H_
#define LOGPOOL_STRING_H_

typedef struct buffer {
    char *buf;
    char *ebuf;
    void *unused;
    char base[1];
} buffer_t;

void *logpool_string_init(logctx ctx, void *param);
void logpool_string_null(logctx ctx, const char *key, uint64_t v);
void logpool_string_bool(logctx ctx, const char *key, uint64_t v);
void logpool_string_int(logctx ctx, const char *key, uint64_t v);
void logpool_string_hex(logctx ctx, const char *key, uint64_t v);
void logpool_string_float(logctx ctx, const char *key, uint64_t v);
void logpool_string_char(logctx ctx, const char *key, uint64_t v);
void logpool_string_string(logctx ctx, const char *key, uint64_t v);
void logpool_string_raw(logctx ctx, const char *key, uint64_t v);
void logpool_string_delim(logctx ctx);
void logpool_string_flush(logctx ctx);

#endif /* end of include guard */
