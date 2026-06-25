// LED Blinking — FreeRTOS Task Version
// Instead of toggling the LED inside loop() with delay(), the blink
// behavior lives in its own task with its own stack and priority.
// vTaskDelay() yields the core to other tasks for the delay duration,
// rather than the task occupying CPU time the way a busy-wait would.

#include <Arduino.h>

#define LED_PIN 4

void ledTask(void *parameter) {
  pinMode(LED_PIN, OUTPUT);
  while (true) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup() {
  // Params: function, name, stack size (words), arg, priority, handle
  xTaskCreate(ledTask, "LED Blink Task", 1024, NULL, 1, NULL);
}

void loop() {
  // loop() itself is just the default Arduino task — leave it idle.
  // All real work happens inside ledTask().
  vTaskDelay(pdMS_TO_TICKS(1000));
}