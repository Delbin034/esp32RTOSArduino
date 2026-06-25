// Button Interfacing — FreeRTOS Task Version
// The ISR does the absolute minimum: it notifies a waiting task.
// All real work (debounce + toggling the LED) happens inside the task,
// which stays blocked on ulTaskNotifyTake() using zero CPU until the
// ISR wakes it — no polling, no flag checked every loop() iteration.
/*Designed and Created By Delbin*/
#include <Arduino.h>

#define BUTTON_PIN 34
#define LED_PIN 5

TaskHandle_t buttonTaskHandle = NULL;

void IRAM_ATTR buttonISR() {
  // Wakes buttonTask; safe to call from an ISR, does no blocking work itself.
  vTaskNotifyGiveFromISR(buttonTaskHandle, NULL);
}

void buttonTask(void *parameter) {
  bool ledState = false;
  pinMode(LED_PIN, OUTPUT);

  while (true) {
    // Block here (using no CPU) until the ISR sends a notification
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Debounce: wait out the bounce window, then confirm the pin is
    // still in the pressed state before acting on it
    vTaskDelay(pdMS_TO_TICKS(30));
    if (digitalRead(BUTTON_PIN) == LOW) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  xTaskCreate(buttonTask, "Button Task", 2048, NULL, 2, &buttonTaskHandle);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}