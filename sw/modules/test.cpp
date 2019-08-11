#include "test.h"
extern "C" {
    #include "node.h"
}

void Test::blaa()
{
    node_debug(LOG_LEVEL_DEBUG, "asd", "test:blaa");
}