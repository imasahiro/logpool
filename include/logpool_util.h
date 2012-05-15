#ifndef LOGPOOL_UTIL_H
#define LOGPOOL_UTIL_H

/* @see plugin/lio_plugin.c */
logpool_t *logpool_open_client(logpool_t *parent, char *host, int port);
int logpoold_start(char *host, int port);
void logpool_query(logpool_t *logpool, char *q);
void *logpool_client_get(logpool_t *logpool, void *buff, size_t bufsize);

#endif /* end of include guard */
