#include <Arduino.h>

// #define LGFX_WYWY_ESP32S3_HMI_DEVKIT       // wywy ESP32S3 HMI DevKit

// #define LGFX_AUTODETECT 

#include <LovyanGFX.hpp>

#include <LGFX_AUTODETECT.hpp> 

#include <lvgl.h>
#include "../lib/lv_lib_qrcode/lv_qrcode.h"


#define GFX_BL 38

#define TFT_WIDTH 480
#define TFT_HEIGHT 480

#define PIN_PANEL_CLK 48
#define PIN_PANEL_LAT 18
#define PIN_PANEL_OE 39
#define PIN_PANEL_A 16
#define PIN_PANEL_B 17
#define PIN_PANEL_C 21

#define PIN_PANEL_R1 11
#define PIN_PANEL_G1 8
#define PIN_PANEL_B1 4
#define PIN_PANEL_R2 12
#define PIN_PANEL_G2 20
#define PIN_PANEL_B2 5
#define PIN_PANEL_R3 13
#define PIN_PANEL_G3 3
#define PIN_PANEL_B3 6
#define PIN_PANEL_R4 14
#define PIN_PANEL_G4 46
#define PIN_PANEL_B4 7


class LGFX : public lgfx::LGFX_Device
{
    // provide an instance that matches the type of panel you want to connect to.
    lgfx::Panel_ST7796 _panel_instance;

    // provide an instance that matches the type of bus to which the panel is connected.
    lgfx::Bus_SPI _bus_instance; // Instances of spi buses

    //Prepare an instance that matches the type of touchscreen. 
    lgfx::Touch_FT5x06  _touch_instance;

    lgfx::Light_PWM     _light_instance;

public:

  LGFX(void)
  {
    {
      // set up bus control.
      auto cfg = _bus_instance.config(); // gets the structure for bus settings.

      // SPI bus settings
      cfg.spi_host = SPI2_HOST; // Select SPI to use ESP32-S2,C3: SPI2_HOST or SPI3_HOST / ESP32: VSPI_HOST or HSPI_HOST

      //* Due to the ESP-IDF upgrade, the description of VSPI_HOST , HSPI_HOST will be deprecated, so if you get an error, use SPI2_HOST , SPI3_HOST instead.
      cfg.spi_mode = 0;          // Set SPI communication mode (0-3) 
      cfg.freq_write = 80000000; // SPI clock on transmission (up to 80MHz, rounded to 80MHz divided by integer)
      cfg.freq_read = 16000000;  // SPI clock on reception
      cfg.spi_3wire = true;      // Set true when receiving on the MOSI pin
      cfg.use_lock = true;       // set true if transaction lock is used
 
      //  * With the ESP-IDF version upgrade, SPI_DMA_CH_AUTO (automatic setting) of DMA channels is recommended. 
      cfg.dma_channel = SPI_DMA_CH_AUTO; // Set DMA channel to be used (0=DMA not used / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=Auto setting)

      cfg.pin_sclk = TFT_SCLK; // Set SCLK pin number for SPI
      cfg.pin_mosi = TFT_MOSI; // Set SPI MOSI pin number

      // When using the SPI bus, which is common to the SD card, be sure to set MISO without omitting it.
      cfg.pin_miso = TFT_MISO; // Set THE MSO pin number of spi (-1 = disable)
      cfg.pin_dc = TFT_DC;    // Set THE D/C pin number of SPI (-1 = disable)

      _bus_instance.config(cfg);              // reflects the setting value on the bus.
      _panel_instance.setBus(&_bus_instance); // Set the bus to the panel.
    }

    {
      // set the display panel control.
      auto cfg = _panel_instance.config(); // gets the structure for display panel settings.

      cfg.pin_cs = TFT_CS;    // Pin number to which CS is connected (-1 = disable)
      cfg.pin_rst = TFT_RST;  // Pin number to which RST is connected (-1 = disable)
      cfg.pin_busy = -1;      // Pin number to which BUSY is connected (-1 = disable)

      // the following setting values are set to a general initial value for each panel,
      cfg.panel_width = TFT_WIDTH;    // actual visible width
      cfg.panel_height = TFT_HEIGHT;   // actually visible height
      cfg.offset_x = 0;         // Panel X-direction offset amount
      cfg.offset_y = 0;         // Panel Y offset amount
      cfg.offset_rotation = 0;  // offset of rotational values from 0 to 7 (4 to 7 upside down)
      cfg.dummy_read_pixel = 8; // number of bits in dummy leads before pixel read
      cfg.dummy_read_bits = 1;  // number of bits in dummy leads before reading non-pixel data
      cfg.readable = false;      // set to true if data can be read
      cfg.invert = false;       // set to true if the light and dark of the panel is reversed
      cfg.rgb_order = false;    // set to true if the red and blue of the panel are swapped
      cfg.dlen_16bit = false;   // Set to true for panels that transmit data lengths in 16-bit increments in 16-bit parallel or SPI
      cfg.bus_shared = true;    // Set to true when sharing the bus with sd card (bus control is performed with drawJpgFile, etc.)

      // The following should only be set if the display is misalized by a driver with a variable number of pixels, such as st7735 or ILI9163.
      // cfg.memory_width = 240; //Maximum width supported by driver ICs
      // cfg.memory_height = 320; //Maximum height supported by driver ICs

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();    

      cfg.pin_bl = 23;//45;              
      cfg.invert = false;           
      cfg.freq   = 44100;           
      cfg.pwm_channel = 7;          

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  
    }

    // { 
    //   auto cfg = _touch_instance.config();

    //   cfg.x_min      = 0;
    //   cfg.x_max      = 319;
    //   cfg.y_min      = 0;  
    //   cfg.y_max      = 479;
    //   cfg.pin_int    = TOUCH_INT;  
    //   cfg.bus_shared = false; 
    //   cfg.offset_rotation = 0;

    //   cfg.i2c_port = 1;//I2C_NUM_1;
    //   cfg.i2c_addr = 0x38;
    //   cfg.pin_sda  = TOUCH_SDA;   
    //   cfg.pin_scl  = TOUCH_SCL;   
    //   cfg.freq = 400000;  

    //   _touch_instance.config(cfg);
    //   _panel_instance.setTouch(&_touch_instance);  
    // }

    setPanel(&_panel_instance); // Set the panel to be used.
  }
};


static LGFX lcd;                 // LGFXのインスタンスを作成。
static LGFX_Sprite sprite(&lcd);

/* Change to your screen resolution */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;
static lv_obj_t * win;


void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      lcd.drawPixel(x, y, color_p->full);
      color_p++;
    }
  }

  lv_disp_flush_ready(disp);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("LVGL Widgets Demo");

  // st.setPanel(&lcd.rgb888);
  // st.init();

  

  lcd.init();
  lcd.setRotation(0);

  lcd.setColorDepth(24);
  // lcd.setPanelColor(0x000000);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  lv_init();

  disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * lcd.width() * 200);
  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, lcd.width() * 200);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = lcd.width();
    disp_drv.ver_res = lcd.height();
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    win = lv_win_create(lv_scr_act(), 0);
    LV_IMG_DECLARE(um6p_qr);
    lv_obj_t * avatar = lv_img_create(win);
    lv_img_set_src(avatar, &um6p_qr);
    

    // lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    // lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);

    // lv_obj_t *qr = lv_qrcode_create(lv_scr_act(), 150, fg_color, bg_color);

    // const char *data = "https://lvgl.io";
    // lv_qrcode_update(qr, data, strlen(data));
    // lv_obj_center(qr);

    // lv_obj_set_style_border_color(qr, bg_color, 0);
    // lv_obj_set_style_border_width(qr, 5, 0);

    Serial.println("Setup done");
  }
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
