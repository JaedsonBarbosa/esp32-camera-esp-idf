#include "driver/gpio.h"
#include "pin.h"
#include "hal/gpio_hal.h"

esp_err_t customPinMode(uint8_t pin, uint8_t mode) {
  if (!GPIO_IS_VALID_GPIO(pin)) {
    return ESP_ERR_INVALID_ARG;
  }

  gpio_hal_context_t gpiohal;
  gpiohal.dev = GPIO_LL_GET_HW(GPIO_PORT_0);

  gpio_config_t conf = {
    /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    .pin_bit_mask = (1ULL << pin),
    .mode = GPIO_MODE_DISABLE,         /*!< GPIO mode: set input/output mode */
    .pull_up_en = GPIO_PULLUP_DISABLE, /*!< GPIO pull-up     */
    .pull_down_en = GPIO_PULLDOWN_DISABLE, /*!< GPIO pull-down */
    .intr_type = (gpio_int_type_t)(gpiohal.dev->pin[pin].int_type
    ) /*!< GPIO interrupt type - previously set                 */
  };
  if (mode < 0x20) { // io
    conf.mode = (gpio_mode_t)(mode & (INPUT | OUTPUT));
    if (mode & OPEN_DRAIN) {
      conf.mode = (gpio_mode_t)(conf.mode | GPIO_MODE_DEF_OD);
    }
    if (mode & PULLUP) {
      conf.pull_up_en = GPIO_PULLUP_ENABLE;
    }
    if (mode & PULLDOWN) {
      conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    }
  }
  esp_err_t result = gpio_config(&conf);
  return result;
}
