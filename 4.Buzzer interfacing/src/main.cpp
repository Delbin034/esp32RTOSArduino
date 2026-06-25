// Buzzer Interfacing — FreeRTOS Task Version
// A binary semaphore lets any other task "request a beep" without
// waiting for it to finish or knowing anything about how the tone
// is generated. The buzzer task blocks on the semaphore using no
// CPU until something gives it.

#include <Arduino.h>

#define BUZZER_PIN 5
#define BUZZER_CHANNEL 0

SemaphoreHandle_t buzzerSemaphore;

void buzzerTask(void *parameter) {
  while (true) {
    xSemaphoreTake(buzzerSemaphore, portMAX_DELAY);

    ledcSetup(BUZZER_CHANNEL, 1000, 8);      // 1 kHz, 8-bit
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    ledcWrite(BUZZER_CHANNEL, 128);          // 50% duty
    vTaskDelay(pdMS_TO_TICKS(300));

    ledcWrite(BUZZER_CHANNEL, 0);            // stop tone
  }
}

// Call this from any other task (button press, alert condition, etc.)
// to request a beep without blocking the caller.
void requestBeep() {
  xSemaphoreGive(buzzerSemaphore);
}

void setup() {
  buzzerSemaphore = xSemaphoreCreateBinary();
  xTaskCreate(buzzerTask, "Buzzer Task", 2048, NULL, 1, NULL);
}

void loop() {
  // Demo trigger — in a real project this call would come from
  // wherever the alert condition actually happens
  requestBeep();
  vTaskDelay(pdMS_TO_TICKS(3000));
}