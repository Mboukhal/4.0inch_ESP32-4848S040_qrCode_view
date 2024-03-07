#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "../lib/lv_lib_qrcode/lv_qrcode.h"
#include <lvgl.h>

#define GFX_BL 38

// void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
//   Serial.println("Connected to AP successfully!");
// }

// void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
// }

// void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
//   Serial.println("Disconnected from WiFi access point");
//   Serial.print("WiFi lost connection. Reason: ");
//   Serial.println(info.wifi_sta_disconnected.reason);
//   Serial.println("Trying to Reconnect");
//   WiFi.begin(SSID, PASSWORD);
// }

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
  39 /* CS */, 48 /* SCK */, 47 /* SDA */,
  18 /* DE */, 17 /* VSYNC */, 16 /* HSYNC */, 21 /* PCLK */,
  11 /* R0 */, 12 /* R1 */, 13 /* R2 */, 14 /* R3 */, 0 /* R4 */,
  8 /* G0 */, 20 /* G1 */, 3 /* G2 */, 46 /* G3 */, 9 /* G4 */, 10 /* G5 */,
  4 /* B0 */, 5 /* B1 */, 6 /* B2 */, 7 /* B3 */, 15 /* B4 */
);
  Arduino_ST7701_RGBPanel *gfx = new Arduino_ST7701_RGBPanel(
  bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */,
  true /* IPS */, 480 /* width */, 480 /* height */,
  st7701_type1_init_operations, sizeof(st7701_type1_init_operations),     true /* BGR */,
  10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
  10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */);


/* Change to your screen resolution */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;
uint16_t new_qrcode = 1;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

  gfx->draw16bitRGBBitmap(area->x1, 
                          area->y1,
                          (uint16_t *)&color_p->full,
                          (area->x2 - area->x1 + 1),
                          (area->y2 - area->y1 + 1));

  lv_disp_flush_ready(disp);
}


// #include <WiFi.h>

// #define SSID "Poco_12"
// #define PASSWORD "12345678"


void setup()
{
  // Serial.begin(115200);

  // delete old config
  // WiFi.disconnect(true);

  // delay(1000);

  // WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  // WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  // WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // WiFi.begin(SSID, PASSWORD);


  // Init Display
  gfx->begin(16000000); /* specify data bus speed */

  gfx->fillScreen(BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  lv_init();


  disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * gfx->width() * 200);
  if (!disp_draw_buf)
  {
    disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * gfx->width() * 200);
    if (!disp_draw_buf)
      return;

  }
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, gfx->width() * 200);

    // /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = gfx->width();
    disp_drv.ver_res = gfx->height();
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

void setNewQRCode(String &data) {
  lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
  lv_color_t fg_color = lv_color_hex(0xEA3B15);


  lv_obj_t * qr = lv_qrcode_create(lv_scr_act(), 480, fg_color, bg_color);

  /*Set data*/
  lv_qrcode_update(qr, data.c_str(), data.length());
  lv_obj_center(qr);
  new_qrcode = 0;
}

void loop()
{
  if (new_qrcode) {
    String data = "https://www.um6p.ma/";
    setNewQRCode(data);

    // Serial.println("QR Code Created");
  }
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
