/*
ESP32 320x240 Capacitive Touch LVGL WIFI & Bluetooth Development Board
ESP32-2432S024
Capacitive touch
*/

// #include "Custom_User_Setup.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "CST820.h"
#include "lv_conf.h"

// Screen Dimensions
#define WIDTH 320
#define HEIGHT 240
#define ROTATION 3

// Touch PINS
#define I2C_SDA 33
#define I2C_SCL 32
#define TP_RST 25
#define TP_INT 21

// Backlight PIN
#define BACKLIGHT 27

// LED PINS
#define LED_RED 4
#define LED_BLUE 17
#define LED_GREEN 16

// Display LVGL Buffers
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1;
static lv_color_t *buf2;

TFT_eSPI tft = TFT_eSPI();
CST820 touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  // tft.startWrite();
  tft.pushImageDMA(area->x1, area->y1, w, h, (uint16_t *)color_p);
  // tft.endWrite();
  lv_disp_flush_ready(disp);
}

// Capacitive Touchpad Read routine
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  bool touched;
  uint8_t gesture;
  uint16_t touchX, touchY;
  touched = touch.getTouch(&touchX, &touchY, &gesture);

  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
    Serial.println((String) "Touch Data x: " + touchX + (String) ", y: " + touchY);
  }
}

//_______________________
void lv_example_btn(void)
{
  /*要转换的属性*/
  static lv_style_prop_t props[] = {
      LV_STYLE_TRANSFORM_WIDTH, LV_STYLE_TRANSFORM_HEIGHT, LV_STYLE_TEXT_LETTER_SPACE};

  /*Transition descriptor when going back to the default state.
   *Add some delay to be sure the press transition is visible even if the press was very short*/
  static lv_style_transition_dsc_t transition_dsc_def;
  lv_style_transition_dsc_init(&transition_dsc_def, props, lv_anim_path_overshoot, 250, 100, NULL);

  /*Transition descriptor when going to pressed state.
   *No delay, go to presses state immediately*/
  static lv_style_transition_dsc_t transition_dsc_pr;
  lv_style_transition_dsc_init(&transition_dsc_pr, props, lv_anim_path_ease_in_out, 250, 0, NULL);

  /*Add only the new transition to he default state*/
  static lv_style_t style_def;
  lv_style_init(&style_def);
  lv_style_set_transition(&style_def, &transition_dsc_def);

  /*Add the transition and some transformation to the presses state.*/
  static lv_style_t style_pr;
  lv_style_init(&style_pr);
  lv_style_set_transform_width(&style_pr, 10);
  lv_style_set_transform_height(&style_pr, -10);
  lv_style_set_text_letter_space(&style_pr, 10);
  lv_style_set_transition(&style_pr, &transition_dsc_pr);

  lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -80);
  lv_obj_add_style(btn1, &style_pr, LV_STATE_PRESSED);
  lv_obj_add_style(btn1, &style_def, 0);

  lv_obj_t *label = lv_label_create(btn1);
  lv_label_set_text(label, "btn1");

  /*Init the style for the default state*/
  static lv_style_t style;
  lv_style_init(&style);

  lv_style_set_radius(&style, 3);

  lv_style_set_bg_opa(&style, LV_OPA_100);
  lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_bg_grad_color(&style, lv_palette_darken(LV_PALETTE_BLUE, 2));
  lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);

  lv_style_set_border_opa(&style, LV_OPA_40);
  lv_style_set_border_width(&style, 2);
  lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_GREY));

  lv_style_set_shadow_width(&style, 8);
  lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_shadow_ofs_y(&style, 8);

  lv_style_set_outline_opa(&style, LV_OPA_COVER);
  lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_BLUE));

  lv_style_set_text_color(&style, lv_color_white());
  lv_style_set_pad_all(&style, 10);

  /*Init the pressed style*/
  static lv_style_t style_pr_2;
  lv_style_init(&style_pr_2);

  /*Ad a large outline when pressed*/
  lv_style_set_outline_width(&style_pr_2, 30);
  lv_style_set_outline_opa(&style_pr_2, LV_OPA_TRANSP);

  lv_style_set_translate_y(&style_pr_2, 5);
  lv_style_set_shadow_ofs_y(&style_pr_2, 3);
  lv_style_set_bg_color(&style_pr_2, lv_palette_darken(LV_PALETTE_BLUE, 2));
  lv_style_set_bg_grad_color(&style_pr_2, lv_palette_darken(LV_PALETTE_BLUE, 4));

  /*Add a transition to the the outline*/
  static lv_style_transition_dsc_t trans;
  static lv_style_prop_t props2[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA};
  lv_style_transition_dsc_init(&trans, props2, lv_anim_path_linear, 300, 0, NULL);

  lv_style_set_transition(&style_pr_2, &trans);

  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
  lv_obj_remove_style_all(btn2); /*Remove the style coming from the theme*/
  lv_obj_add_style(btn2, &style, 0);
  lv_obj_add_style(btn2, &style_pr_2, LV_STATE_PRESSED);
  lv_obj_set_size(btn2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_center(btn2);

  lv_obj_t *label2 = lv_label_create(btn2);
  lv_label_set_text(label2, "Button");
  lv_obj_center(label2);
}
//_______________________

void setup()
{
  Serial.begin(115200); // Start Serial Monitor

  Serial.println("Initiating Board");
  Serial.println((String) "Ver. " + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch());

  lv_init();

  // Set RGB LED
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Turn off RGB LED
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);

  tft.begin();               // Initiate TFT Scree
  tft.setRotation(ROTATION); // Set screen rotation
  tft.initDMA();             // Initiate DMA

  touch.begin(WIDTH, HEIGHT, ROTATION); // Initiate Capacitive Touch

  // Set Backlight
  pinMode(BACKLIGHT, OUTPUT);
  analogWrite(BACKLIGHT, 255);

  // Test Screen
  tft.fillScreen(TFT_RED);
  digitalWrite(LED_RED, LOW);
  delay(250);
  tft.fillScreen(TFT_GREEN);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  delay(250);
  tft.fillScreen(TFT_BLUE);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, LOW);
  delay(250);
  tft.fillScreen(TFT_BLACK);
  digitalWrite(LED_BLUE, HIGH);
  delay(250);

  buf1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * WIDTH * 100, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  buf2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * WIDTH * 100, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

  /* Initialize Display */
  static lv_disp_drv_t disp_drv;
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, WIDTH * 100);
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = WIDTH;
  disp_drv.ver_res = HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize (virtual) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  lv_example_btn();

  Serial.println("Setup done");
  tft.startWrite();
}

void loop()
{
  lv_timer_handler(); /* Display Routine */
  delay(5);
}
