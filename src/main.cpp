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

// Создание интерфейса
void create_ui() {
  lv_obj_t *cont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(cont, WIDTH, HEIGHT);
  lv_obj_set_style_pad_all(cont, 10, 0);

  label_request = lv_label_create(cont);
  lv_label_set_text(label_request, "Request:");
  lv_obj_align(label_request, LV_ALIGN_TOP_MID, 0, 20);

  label_response = lv_label_create(cont);
  lv_label_set_text(label_response, "Response:");
  lv_obj_align(label_response, LV_ALIGN_CENTER, 0, 20);

  progress_bar = lv_bar_create(cont);
  lv_obj_set_size(progress_bar, WIDTH - 40, 10);
  lv_bar_set_range(progress_bar, 0, 255);
  lv_obj_align(progress_bar, LV_ALIGN_BOTTOM_MID, 0, -40);
}

// Обновление интерфейса
void update_ui(const char *request, const char *response, uint8_t checksum) {
  lv_label_set_text(label_request, request);
  lv_label_set_text(label_response, response);
  lv_bar_set_value(progress_bar, checksum, LV_ANIM_ON);
}

// Отправка запроса
void send_request(uint8_t *request, size_t length) {
  Serial2.write(request, length);
  Serial.print("Request sent: ");
  for (size_t i = 0; i < length; i++) {
    Serial.printf("%02X ", request[i]);
  }
  Serial.println();
}

// Получение ответа
bool receive_response(uint8_t *response, size_t length, uint32_t timeout) {
  size_t received = 0;
  unsigned long start_time = millis();

  while (millis() - start_time < timeout) {
    while (Serial2.available() > 0 && received < length) {
      response[received++] = Serial2.read();
    }
    if (received >= length) return true;
    lv_task_handler();  // Обновление интерфейса
    delay(10);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(PRESSURE_BAUD, SERIAL_8N1, PRESSURE_RX, PRESSURE_TX);

  tft.begin();
  tft.setRotation(ROTATION);
  tft.fillScreen(TFT_BLACK);

  uint16_t calData[5] = {355, 3650, 325, 3600, 7};
  tft.setTouch(calData);

  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);

  Serial.println("Program started");

  // Инициализация LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, WIDTH * 10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = WIDTH;
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

  create_ui();
}

void loop() {
  uint8_t request[5] = {0x02, 0x02, 0x01, 0x00, 0x01};
  uint8_t response[10];

  send_request(request, sizeof(request));

  if (receive_response(response, sizeof(response), 10000)) {
    if (calculate_checksum(response, 9) == response[9]) {
      Serial.print("Response received: ");
      for (size_t i = 0; i < 10; i++) {
        Serial.printf("%02X ", response[i]);
      }
      Serial.println();

      char response_text[50];
      snprintf(response_text, sizeof(response_text),
               "Response: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
               response[0], response[1], response[2], response[3], response[4],
               response[5], response[6], response[7], response[8], response[9]);

      char request_text[50];
      snprintf(request_text, sizeof(request_text),
               "Request: %02X %02X %02X %02X %02X",
               request[0], request[1], request[2], request[3], request[4]);

      update_ui(request_text, response_text, response[9]);
    } else {
      Serial.println("Checksum error");
      update_ui("Request sent", "Checksum error", 0);
    }
  } else {
    Serial.println("No response received within 10 seconds.");
    update_ui("Request sent", "No response", 0);
  }

  lv_task_handler();  // Обработка задач LVGL
}
