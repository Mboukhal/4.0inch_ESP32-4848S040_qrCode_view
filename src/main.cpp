#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "../lib/lv_lib_qrcode/lv_qrcode.h"
#include <lvgl.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define SSID "Poco_12"
#define PASSWORD "12345678"


const String MQTT_SERVER = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const String MQTT_CLIENT_ID = "mqttx_11101088";


// #define MQTT_AUTH_USER ""
// #define MQTT_AUTH_PASSWORD ""

#define TOPIC_NEW_QR_CODE "um6p/new_qrcode"
#define TOPIC_GET_QR_CODE "um6p/get_qrcode"


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

#define GFX_BL 38

/* Change to your screen resolution */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;
String QR_CODE = "Wait For Connection!";
bool isQrCode = false;


WiFiClient mqttClient;
PubSubClient client(mqttClient);


#define LV_COLOR_WIHTE LV_COLOR_MAKE(0xff, 0xff, 0xff)
#define LV_COLOR_BLACK LV_COLOR_MAKE(0x00, 0x00, 0x00)

void display_wait(String message) {
  /*Create a style for the shadow*/

  lv_obj_clean(lv_scr_act());

  lv_obj_t *background = lv_obj_create(lv_scr_act());
  lv_obj_set_size(background, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_set_style_bg_color(background, LV_COLOR_WIHTE, 0);

  lv_obj_t * label = lv_label_create(background);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
  lv_label_set_recolor(label, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text(label, message.c_str());
  lv_obj_set_width(label, 400);  /*Set smaller width to make the lines wrap*/

  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  lv_timer_handler();
}

/* try to connect to the wifi, if not connected, try again */
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){

  isQrCode = false;
  WiFi.disconnect();

  String message = "#000000 <# #366e11 Future:# #000000 Is_Loading />#";
  display_wait(message);
  Serial.print(".");
  WiFi.begin(SSID, PASSWORD);
  delay(500);


}

void setNewQRCode(void) {

  // lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
  lv_color_t bg_color = lv_color_hex(0x000);;
  lv_color_t fg_color = lv_color_hex(0xEA3B15);

  lv_obj_clean(lv_scr_act());

  lv_obj_t * qr = lv_qrcode_create(lv_scr_act(), 480, fg_color, bg_color);
  lv_qrcode_update(qr, QR_CODE.c_str(), QR_CODE.length());
  lv_obj_center(qr);
  isQrCode = true;
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {

  // gfx->fillScreen(0x0000);

  gfx->draw16bitRGBBitmap(area->x1 + 1, 
      area->y1 + 1,
      (uint16_t *)&color_p->full,
      (area->x2 - area->x1 + 1),
      (area->y2 - area->y1 + 1));


  lv_disp_flush_ready(disp);
}

void wifi_init_sta(void) {
  // delete old config
  WiFi.disconnect(true);
  delay(50);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.begin(SSID, PASSWORD);

  // Serial.print("Connecting to WiFi...");
  
}

void callback(char *topic, uint8_t *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    Serial.println();
    Serial.println("-----------------------");
    if (String(topic) == TOPIC_NEW_QR_CODE) {
      QR_CODE = "";
      for (int i = 0; i < length; i++) {
        QR_CODE += (char) payload[i];
        Serial.print((char) payload[i]);
      }
      setNewQRCode();
      isQrCode = false;
    
    }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED)
      return;
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect

    // if (client.connect(MQTT_CLIENT_ID.c_str()), MQTT_AUTH_USER, MQTT_AUTH_PASSWORD) {
    if (client.connect(MQTT_CLIENT_ID.c_str())) {
      Serial.print("Connected, subscribing to: ");
      Serial.println(TOPIC_NEW_QR_CODE);
      // Subscribe
      // Check if subscription was successful
      if (client.subscribe(TOPIC_NEW_QR_CODE)) {
        Serial.println("Subscribed successfully.");
      } else {
        Serial.println("Subscription failed!");
        // Handle subscription failure (e.g., retry)
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  // TODO: try to cennect to API init and send the serial number

  // Init Display
  gfx->begin(16000000); 

  wifi_init_sta();

  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  gfx->fillScreen(BLACK);

  lv_init();
  disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * ((gfx->width() * 200) + 1));
  if (!disp_draw_buf) {
    disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * ((gfx->width() * 200) + 1));
    if (!disp_draw_buf)
      return;
  }
  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, (gfx->width() * 200) + 1);
  // /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = gfx->width();
  disp_drv.ver_res = gfx->height();
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  String message = "#000000 <# #366e11 Future:# #000000 Is_Loading />#";
  display_wait(message);


  while (WiFi.status() != WL_CONNECTED)
    delay(50);

  client.setServer(MQTT_SERVER.c_str(), MQTT_PORT);  
  client.setCallback(callback);
  // client.connect(MQTT_CLIENT_ID.c_str());
  reconnect();

  client.publish(TOPIC_GET_QR_CODE, MQTT_CLIENT_ID.c_str());
  
}

void loop() {

  if (client.connected() && WiFi.status() == WL_CONNECTED) {

      // client.publish(TOPIC_NEW_QR_CODE, "OK");
      // Serial.println("Published");
    // TODO: onEvent get new qr code string and set  from the server and set it to the `QR_CODE`
    if (!isQrCode)
      setNewQRCode();
      // Serial.println("QR Code Created");
    // } 
  }
  client.loop();
  lv_timer_handler(); /* let the GUI do its work */
  delay(500);
}
