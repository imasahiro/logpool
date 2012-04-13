#include "logpoold.h"

#define SERVER_IP     "127.0.0.1"
#define SERVER_PORT    10000

using namespace logpool;
int main(int argc, char const* argv[])
{
    (void)argc;(void)argv;
    lpevent ev(SERVER_IP, SERVER_PORT);
    ev.exec();
    return 0;
}
