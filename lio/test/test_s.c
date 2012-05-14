#include "lio.h"

int main(int argc, char **argv)
{
    extern struct lio_api server_api;
    struct lio *lio = lio_open("127.0.0.1", 14801, LIO_MODE_READ|LIO_MODE_WRITE, &server_api);
    return lio_dispatch(lio);
}
