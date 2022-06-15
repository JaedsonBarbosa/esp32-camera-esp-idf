#include "OV7670.h"
#include <esp_log.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>

#define CAM_PIN_PWDN 2   // power down is not used
#define CAM_PIN_RESET 15 // software reset will be performed
#define CAM_PIN_XCLK 26
#define CAM_PIN_SIOD 22 // SDA
#define CAM_PIN_SIOC 21 // SCL

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 33
#define CAM_PIN_D4 32
#define CAM_PIN_D3 19
#define CAM_PIN_D2 18
#define CAM_PIN_D1 5
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 27

esp_err_t configure_reset_pwdn() {
  gpio_config_t io_conf;
  io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;//disable interrupt
  io_conf.mode = GPIO_MODE_OUTPUT;//set as output mode
  io_conf.pin_bit_mask = (1ULL<<CAM_PIN_RESET);//bit mask of the pins that you want to set,e.g.GPIO18
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;//disable pull-down mode
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;//disable pull-up mode
  esp_err_t error=gpio_config(&io_conf);//configure GPIO with the given settings
  if(error!=ESP_OK){
    printf("error configuring reset \n");
    return ESP_FAIL;
  }
  gpio_set_level((gpio_num_t)CAM_PIN_RESET, 1);

  io_conf.pin_bit_mask = (1ULL<<CAM_PIN_PWDN);
  error=gpio_config(&io_conf);
  if(error!=ESP_OK){
    printf("error configuring pwdn \n");
    return ESP_FAIL;
  }
  gpio_set_level((gpio_num_t)CAM_PIN_PWDN, 0);

  return ESP_OK;
}

void test_capture() {
  OV7670 *camera = new OV7670(
    OV7670::Mode::QQVGA_RGB565,
    CAM_PIN_SIOD,
    CAM_PIN_SIOC,
    CAM_PIN_VSYNC,
    CAM_PIN_HREF,
    CAM_PIN_XCLK,
    CAM_PIN_PCLK,
    CAM_PIN_D0,
    CAM_PIN_D1,
    CAM_PIN_D2,
    CAM_PIN_D3,
    CAM_PIN_D4,
    CAM_PIN_D5,
    CAM_PIN_D6,
    CAM_PIN_D7
  );
  ESP_LOGI("MAIN", "Instanciado.");
  int blk_count = camera->yres / I2SCamera::blockSlice; // 30, 60, 120
  for (int i = 0; i < blk_count; i++) {

    if (i == 0) {
      camera->startBlock = 1;
      camera->endBlock = I2SCamera::blockSlice;
      // webSocket.sendBIN(0, &start_flag, 1);
    }

    if (i == blk_count - 1) {
      // webSocket.sendBIN(0, &end_flag, 1);
    }

    camera->oneFrame();
    // webSocket.sendBIN(0, camera->frame, camera->xres * I2SCamera::blockSlice
    // * 2);
    vTaskDelay(pdMS_TO_TICKS(1));
    camera->startBlock += I2SCamera::blockSlice;
    camera->endBlock += I2SCamera::blockSlice;
  }
}

// void app_main(void)
extern "C" void app_main() {
  esp_task_wdt_init(60, false);
  esp_err_t error = configure_reset_pwdn();
  if (error != ESP_OK) {
    printf("error configuring inputs \n");
    return;
  }
  test_capture();
  ESP_LOGI("MAIN", "Sucesso");
}
