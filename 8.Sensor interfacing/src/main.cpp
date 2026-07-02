// Sensor Interfacing (LM35, LDR, MQ series) — FreeRTOS Task Version
// This is where RTOS-based design actually pays off for analog
// sensing: three sensors with very different timing needs run as
// three independent tasks. The MQ sensor's multi-second warm-up
// delay does not block the LM35 or LDR tasks at all — something
// that would be awkward to express cleanly in a single loop().
//
// All three pins are on ADC1 (GPIO32-39) to avoid the ADC2/Wi-Fi
// conflict described in Chapter 2.

#include <Arduino.h>

#define LM35_PIN 34
#define LDR_PIN  35
#define MQ_PIN   32
#define MQ_WARMUP_MS 60000UL // shortened for example; many MQ sensors specify several minutes

void lm35Task(void *parameter) {
  while (true) {
    int raw = analogRead(LM35_PIN);
    float voltage = (raw / 4095.0) * 3.3;
    float tempC = voltage * 100.0; // LM35: 10mV per degree C
    Serial.printf("[LM35] Temperature: %.1f C\n", tempC);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void ldrTask(void *parameter) {
  while (true) {
    int raw = analogRead(LDR_PIN);
    Serial.printf("[LDR] Light level (raw ADC): %d\n", raw);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void mqTask(void *parameter) {
  unsigned long startTime = millis();
  while (millis() - startTime < MQ_WARMUP_MS) {
    Serial.println("[MQ] Warming up...");
    vTaskDelay(pdMS_TO_TICKS(2000)); // other tasks keep running normally during this wait
  }
  while (true) {
    int raw = analogRead(MQ_PIN);
    Serial.printf("[MQ] Gas sensor raw reading: %d\n", raw);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  xTaskCreate(lm35Task, "LM35 Task", 2048, NULL, 1, NULL);
  xTaskCreate(ldrTask, "LDR Task", 2048, NULL, 1, NULL);
  xTaskCreate(mqTask, "MQ Task", 2048, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}