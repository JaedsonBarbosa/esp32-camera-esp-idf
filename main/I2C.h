#pragma once

#include "freertos/FreeRTOS.h"
#include "rom/ets_sys.h"
#include "pin.h"

class I2C {
  void inline DELAY() { ets_delay_us(1); }

  void inline SCLLOW() {
    customPinMode(SCL, OUTPUT);
    gpio_set_level((gpio_num_t)SCL, 0);
  }

  void inline SCLHIGH() {
    customPinMode(SCL, INPUT_PULLUP);
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
    customPinMode(SDA, OUTPUT);
    gpio_set_level((gpio_num_t)SDA, 0);
  }

  void inline SDAHIGH() {
    customPinMode(SDA, OUTPUT);
    gpio_set_level((gpio_num_t)SDA, 1);
  }

  void inline SDAPULLUP() { customPinMode(SDA, INPUT_PULLUP); }

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
    customPinMode(SDA, INPUT_PULLUP);
    customPinMode(SCL, INPUT_PULLUP);
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
