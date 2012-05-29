#include "logpool.h"
#include "logpool_util.h"

int main(int argc, char **argv)
{
    return logpoold_start("127.0.0.1", 14801);
}
