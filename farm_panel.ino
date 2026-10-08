// FARM-H1 제어판 (PF 0.0.3 = 3단계: 홈 화면 틀 + 가짜 값)
// 바탕: Waveshare 09_lvgl_Porting 샘플. 보드를 켜는 부분은 샘플 그대로이고,
// 화면은 ui_home.h 가 그린다. 값 틀은 farm_model.h, 큰 숫자 글꼴은 font_num_44.c,
// 한글 글꼴은 font_kr_20.c.
#include <Arduino.h>
#include <esp_display_panel.hpp>

#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "farm_model.h"
#include "ui_home.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

void setup()
{
    Serial.begin(115200);

    Serial.println("Initializing board");
    Board *board = new Board();
    board->init();

    #if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    // When avoid tearing function is enabled, the frame buffer number should be set in the board driver
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    /**
     * As the anti-tearing feature typically consumes more PSRAM bandwidth, for the ESP32-S3, we need to utilize the
     * "bounce buffer" functionality to enhance the RGB data bandwidth.
     * This feature will consume `bounce_buffer_size * bytes_per_pixel * 2` of SRAM memory.
     */
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
#endif
#endif
    assert(board->begin());

    Serial.println("Initializing LVGL");
    lvgl_port_init(board->getLCD(), board->getTouch());

    Serial.println("Creating UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    ui_home_create();

    /* Release the mutex */
    lvgl_port_unlock();
}

void loop()
{
    Serial.println("IDLE loop");
    delay(1000);
}
