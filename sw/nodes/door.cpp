#include "modules/test.h"
#include "modules/BLAKE2s/BLAKE2s.h"

extern "C" {
    #include "node.h"
}

void test(void) {
    // Test test;
    // test.blaa();
    node_debug(LOG_LEVEL_DEBUG, "asd", "Hello World++");
}

// // hash out of secret & uid
// // to get an unpredictable password, which is unique for each tag id
// // also used for generating the pACK and the stored data
void getHash(uint8_t *out, char *secret, size_t secretlen) {
  BLAKE2s blake2s;
  blake2s.reset(secret, secretlen, 32);
  blake2s.update("asdasd", 3);
  blake2s.finalize(out, 32);
}
char SECRET[] = "devtests";

extern "C" {
    void app_init(void)
    {

    }

    void app_tick(void)
    {

        uint8_t tagKey[32];
        getHash(tagKey, SECRET, sizeof(SECRET));
        test();
    }

    void app_fast_tick(void)
    {

    }

    void app_config_update(void)
    {

    }
}
