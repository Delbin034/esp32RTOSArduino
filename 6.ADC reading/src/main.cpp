// ADC Reading — FreeRTOS Task Version
// The sampling task owns the ADC and runs on its own schedule.
// It shares the latest averaged reading through a single-slot queue
// using xQueueOverwrite() — any consumer task that peeks the queue
// always gets the most recent value, with no risk of reading the
// ADC mid-sample from another task.

#include <Arduino.h>

#define SENSOR_PIN 34
#define SAMPLES 10

QueueHandle_t adcQueue;

void adcReadTask(void *parameter) {
  while (true) {
    long total = 0;
    for (int i = 0; i < SAMPLES; i++) {
      total += analogRead(SENSOR_PIN);
      vTaskDelay(pdMS_TO_TICKS(2));
    }
    int averaged = total / SAMPLES;
    xQueueOverwrite(adcQueue, &averaged); // queue length 1: always holds the latest value
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// Any other task can read the latest value without disturbing it
void consumerTask(void *parameter) {
  int reading;
  while (true) {
    if (xQueuePeek(adcQueue, &reading, pdMS_TO_TICKS(1000)) == pdTRUE) {
      Serial.printf("Latest ADC reading: %d\n", reading);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  adcQueue = xQueueCreate(1, sizeof(int));
  xTaskCreate(adcReadTask, "ADC Read Task", 2048, NULL, 2, NULL);
  xTaskCreate(consumerTask, "Consumer Task", 2048, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}