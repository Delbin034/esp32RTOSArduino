// Voltage Measurement — FreeRTOS Task Version
// This one uses a mutex-protected shared variable instead of a queue,
// as an alternative pattern to the one in the ADC reading example.
// A mutex makes sense here because multiple tasks might eventually
// need to both read AND write related state (not just hand off a
// single value one-directionally, which is what a queue is best at).

#include <Arduino.h>

#define SENSOR_PIN 34
// Divider: R1 = 10k (to source), R2 = 10k (to GND) -> halves input voltage
#define DIVIDER_RATIO 2.0

SemaphoreHandle_t voltageMutex;
volatile float sharedVoltage = 0.0;

void voltageTask(void *parameter) {
  while (true) {
    int raw = analogRead(SENSOR_PIN);
    float vAtPin = (raw / 4095.0) * 3.3;
    float vSource = vAtPin * DIVIDER_RATIO;

    xSemaphoreTake(voltageMutex, portMAX_DELAY);
    sharedVoltage = vSource;
    xSemaphoreGive(voltageMutex);

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void displayTask(void *parameter) {
  while (true) {
    float v;
    xSemaphoreTake(voltageMutex, portMAX_DELAY);
    v = sharedVoltage;
    xSemaphoreGive(voltageMutex);

    Serial.printf("Source voltage: %.2f V\n", v);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  voltageMutex = xSemaphoreCreateMutex();
  xTaskCreate(voltageTask, "Voltage Task", 2048, NULL, 2, NULL);
  xTaskCreate(displayTask, "Display Task", 2048, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}