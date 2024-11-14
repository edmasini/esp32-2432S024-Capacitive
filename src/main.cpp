#include <TFT_eSPI.h>
#include <lvgl.h>

#define WIDTH 320
#define HEIGHT 240
#define ROTATION 1

#define BACKLIGHT 27
#define PRESSURE_RX 16  // RX пин для UART
#define PRESSURE_TX 17  // TX пин для UART
#define PRESSURE_BAUD 19200

TFT_eSPI tft = TFT_eSPI();

// LVGL дисплейный буфер и драйвер
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[WIDTH * 10];

lv_obj_t *label_request;
lv_obj_t *label_response;
lv_obj_t *progress_bar;

// Функция для расчета контрольной суммы
uint8_t calculate_checksum(uint8_t *data, size_t length) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// Функция для обработки касания с использованием LVGL
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t x, y;
    bool touched = tft.getTouch(&x, &y);

    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
        Serial.printf("Touch detected at x=%d, y=%d\n", x, y);
    }
}

// Функция для обновления отображаемых данных
void update_display_data(const char* request_text, const char* response_text, uint8_t checksum) {
  lv_label_set_text(label_request, request_text);
  lv_label_set_text(label_response, response_text);
  lv_bar_set_value(progress_bar, checksum, LV_ANIM_ON);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(PRESSURE_BAUD, SERIAL_8O1 | UART_PARITY_ODD, PRESSURE_RX, PRESSURE_TX);

  tft.begin();
  tft.setRotation(ROTATION);
  tft.fillScreen(TFT_BLACK);
  
  // Калибровка тачскрина
  uint16_t calData[5] = { 355, 3650, 325, 3600, 7 };  // Данные из примера на GitHub
  tft.setTouch(calData);  // Устанавливаем калибровку

  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);

  Serial.println("Program started");

  // Инициализация LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, WIDTH * 10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = WIDTH;
  disp_drv.ver_res = HEIGHT;
  disp_drv.flush_cb = [](lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
    for (int y = area->y1; y <= area->y2; y++) {
      for (int x = area->x1; x <= area->x2; x++) {
        uint16_t color = color_p->full;
        tft.writeColor(color, 1);
        color_p++;
      }
    }
    tft.endWrite();
    lv_disp_flush_ready(disp_drv);
  };
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Настройка драйвера для тачскрина в LVGL
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // Основной экран (создание интерфейса для запросов)
  lv_obj_t *cont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(cont, WIDTH, HEIGHT);
  lv_obj_set_style_pad_all(cont, 10, 0);

  // Подключение шрифта
  lv_style_t style;
  lv_style_init(&style);
  lv_style_set_text_font(&style, &lv_font_montserrat_14);

  // Создание контейнеров для запроса и ответа
  label_request = lv_label_create(cont);
  lv_label_set_text(label_request, "Request:");
  lv_obj_add_style(label_request, &style, 0);
  lv_obj_align(label_request, LV_ALIGN_TOP_MID, 0, 20);

  label_response = lv_label_create(cont);
  lv_label_set_text(label_response, "Response:");
  lv_obj_add_style(label_response, &style, 0);
  lv_obj_align(label_response, LV_ALIGN_CENTER, 0, 20);

  // Создание круговой диаграммы (progress bar) для контрольной суммы
  progress_bar = lv_bar_create(cont);
  lv_obj_set_size(progress_bar, WIDTH - 40, 20);
  lv_bar_set_range(progress_bar, 0, 255);
  lv_obj_align(progress_bar, LV_ALIGN_BOTTOM_MID, 0, -20);

  delay(1000);
}

void loop() {
  uint8_t request[5] = {0x02, 0x02, 0x01, 0x00, calculate_checksum(request, 4)};
  uint8_t response[8];
  float pressure_value;

  Serial2.write(request, sizeof(request));
  Serial.print("Request sent: ");
  for (int i = 0; i < 5; i++) {
    Serial.printf("%02X ", request[i]);
  }
  Serial.println();

  delay(40);  // Ожидание ответа

  if (Serial2.available() >= 6) {
    Serial2.readBytes(response, 6);
    uint8_t calculated_checksum = calculate_checksum(response, 5);

    if (response[0] == 0x06 && calculated_checksum == response[5]) {
      memcpy(&pressure_value, &response[1], sizeof(float));
      char response_text[50];
      sprintf(response_text, "Pressure: %.2f kPa", pressure_value);
      char request_text[50];
      sprintf(request_text, "Request: %02X %02X %02X %02X %02X", request[0], request[1], request[2], request[3], request[4]);

      update_display_data(request_text, response_text, calculated_checksum);
      Serial.print("Response received: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X ", response[i]);
      }
      Serial.printf("Checksum: %02X\n", calculated_checksum);
    } else {
      update_display_data("Request sent", "Checksum error", 0);
    }
  } else {
    update_display_data("Request sent", "No response from module", 0);
  }

  lv_task_handler(); // LVGL обработка
  delay(5);
}
