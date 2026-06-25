// Digital Sensor Reading (PIR) — FreeRTOS Task Version
// The ISR pushes a timestamp onto a queue and returns immediately.
// A separate task consumes events from the queue at its own pace —
// useful once "what happens on motion" becomes more than a Serial
// print (e.g. turning on a light, publishing an MQTT alert), since
// that work no longer has to fit inside ISR constraints.

#include <Arduino.h>

#define PIR_PIN 34

QueueHandle_t motionQueue;

void IRAM_ATTR pirISR() {
  unsigned long timestamp = millis();
  xQueueSendFromISR(motionQueue, &timestamp, NULL);
}

void motionHandlerTask(void *parameter) {
  unsigned long eventTime;
  while (true) {
    if (xQueueReceive(motionQueue, &eventTime, portMAX_DELAY) == pdTRUE) {
      Serial.printf("Motion detected at t=%lu ms\n", eventTime);
      // Real handling goes here: turn on a light, publish an MQTT
      // message, start a "stay on" timer, etc.
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  motionQueue = xQueueCreate(10, sizeof(unsigned long));
  xTaskCreate(motionHandlerTask, "Motion Handler", 2048, NULL, 2, NULL);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirISR, RISING);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}