// Signal Scaling and Calibration — FreeRTOS Task Version
// The calibration math itself doesn't change — it's the same
// two-point linear calibration from Chapter 4. What's different is
// that calibration now happens in its own task, publishing the
// scaled value to a single-slot queue (xQueueOverwrite) so a
// separate decision task can act on it without ever touching the
// ADC or calibration logic directly.

#include <Arduino.h>

#define SENSOR_PIN 34
// Two-point calibration reference points (raw ADC <-> known physical value)
#define RAW_LOW    200
#define RAW_HIGH   3800
#define VALUE_LOW  0.0
#define VALUE_HIGH 100.0
#define ACTION_THRESHOLD 80.0

QueueHandle_t calibratedQueue;

float calibrate(int raw, int rawLow, int rawHigh, float valueLow, float valueHigh) {
  return valueLow + (float)(raw - rawLow) * (valueHigh - valueLow) / (rawHigh - rawLow);
}

void calibrationTask(void *parameter) {
  while (true) {
    int raw = analogRead(SENSOR_PIN);
    float calibrated = calibrate(raw, RAW_LOW, RAW_HIGH, VALUE_LOW, VALUE_HIGH);
    xQueueOverwrite(calibratedQueue, &calibrated);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void decisionTask(void *parameter) {
  float value;
  while (true) {
    if (xQueuePeek(calibratedQueue, &value, pdMS_TO_TICKS(1000)) == pdTRUE) {
      Serial.printf("Calibrated value: %.2f\n", value);
      if (value > ACTION_THRESHOLD) {
        Serial.println("Threshold exceeded — take action here (relay, alert, etc.)");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  calibratedQueue = xQueueCreate(1, sizeof(float));
  xTaskCreate(calibrationTask, "Calibration Task", 2048, NULL, 2, NULL);
  xTaskCreate(decisionTask, "Decision Task", 2048, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}