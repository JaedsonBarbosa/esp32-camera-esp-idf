#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hal/gpio_hal.h"
#include "rom/ets_sys.h"

#define LOW 0x0
#define HIGH 0x1

// GPIO FUNCTIONS
#define INPUT 0x01
// Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT)
// where you can read the state of pin even when it is set as OUTPUT
#define OUTPUT 0x03
#define PULLUP 0x04
#define INPUT_PULLUP 0x05
#define PULLDOWN 0x08
#define INPUT_PULLDOWN 0x09
#define OPEN_DRAIN 0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define ANALOG 0xC0

class I2C {
  void pinMode(uint8_t pin, uint8_t mode) {
    if (!GPIO_IS_VALID_GPIO(pin)) {
      ESP_LOGE("I2C", "Invalid pin selected");
      return;
    }

    gpio_hal_context_t gpiohal;
    gpiohal.dev = GPIO_LL_GET_HW(GPIO_PORT_0);

    gpio_config_t conf = {
      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
      .pin_bit_mask = (1ULL << pin),
      .mode = GPIO_MODE_DISABLE, /*!< GPIO mode: set input/output mode */
      .pull_up_en = GPIO_PULLUP_DISABLE,     /*!< GPIO pull-up     */
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
    if (gpio_config(&conf) != ESP_OK) {
      ESP_LOGE("I2C", "GPIO config failed");
      return;
    }
  }

  void inline DELAY() { ets_delay_us(1); }

  void inline SCLLOW() {
    pinMode(SCL, OUTPUT);
    gpio_set_level((gpio_num_t)SCL, 0);
  }

  void inline SCLHIGH() {
    pinMode(SCL, INPUT_PULLUP);
    gpio_set_level((gpio_num_t)SCL, 1);
  }

  void inline CLOCK() {
    DELAY();
    SCLHIGH();
    DELAY();
    DELAY();
    SCLLOW();
    DELAY();
  }

  void inline SDALOW() {
    pinMode(SDA, OUTPUT);
    gpio_set_level((gpio_num_t)SDA, 0);
  }

  void inline SDAHIGH() {
    pinMode(SDA, OUTPUT);
    gpio_set_level((gpio_num_t)SDA, 1);
  }

  void inline SDAPULLUP() { pinMode(SDA, INPUT_PULLUP); }

  void pushByte(unsigned char b) {
    for (char i = 0; i < 8; i++) {
      if (b & 0x80)
        SDAHIGH();
      else
        SDALOW();
      b <<= 1;
      CLOCK();
    }
  }

  bool getAck() {
    SDAPULLUP();
    DELAY();
    SCLHIGH();
    DELAY();
    int r = gpio_get_level((gpio_num_t)SDA);
    SDALOW();
    DELAY();
    SCLLOW();
    DELAY();
    return r == 0;
  }

  void start() {
    SDAPULLUP();
    DELAY();
    SCLHIGH();
    DELAY();
    SDALOW();
    DELAY();
    SCLLOW();
    DELAY();
  }

  void end() {
    SCLHIGH();
    DELAY();
    SDAPULLUP();
    DELAY();
  }

public:
  int SDA;
  int SCL;
  I2C(const int data, const int clock) {
    SDA = data;
    SCL = clock;
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);
    gpio_set_level((gpio_num_t)SDA, 0);
    gpio_set_level((gpio_num_t)SCL, 0);
  }

  bool
  writeRegister(unsigned char addr, unsigned char reg, unsigned char data) {
    start();
    pushByte(addr);

    if (!getAck()) {
      end();
      return false;
    }

    pushByte(reg);
    if (!getAck()) {
      end();
      return false;
    }

    pushByte(data);
    if (!getAck()) {
      end();
      return false;
    }

    end();
    return true;
  }
};
