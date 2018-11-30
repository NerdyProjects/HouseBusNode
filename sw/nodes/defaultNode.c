#include "node.h"
#include "modules/bme280.h"

void __attribute__((weak)) app_init(void)
{
  bme280_app_init();
}

void __attribute__((weak)) app_tick(void)
{
  bme280_app_read();

}

void __attribute__((weak)) app_fast_tick(void)
{

}

void __attribute__((weak)) app_config_update(void)
{

}
