#include "query.h"
#include <stdio.h>

int main(int argc, char const* argv[])
{
    int i, j;
    struct query_entry e;
    for (i = 0; i < 10000; ++i) {
        struct query_list *l = query_new();
        //TODO
        //for (j = 0; j < 10000; ++j) {
        //    query_add(l, NULL, &e);
        //}
        query_delete(l);
    }
    return 0;
}
