// Relay Control — FreeRTOS Task Version
// The task that owns the relay GPIO is the only thing that ever calls
// digitalWrite() on it. Anything that wants to change relay state —
// a schedule, an MQTT callback, a sensor threshold — just posts a
// command onto the queue. This is the same decoupled pattern you'd
// use to let an MQTT message control a relay without the MQTT
// callback ever touching the pin directly.

#include <Arduino.h>

#define RELAY_PIN 5
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

enum RelayCommand { RELAY_CMD_ON, RELAY_CMD_OFF };

QueueHandle_t relayQueue;

void relayControlTask(void *parameter) {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF); // safe default state at boot

  RelayCommand cmd;
  while (true) {
    if (xQueueReceive(relayQueue, &cmd, portMAX_DELAY) == pdTRUE) {
      digitalWrite(RELAY_PIN, cmd == RELAY_CMD_ON ? RELAY_ON : RELAY_OFF);
    }
  }
}

// Stand-in for a real command source (MQTT callback, schedule check,
// soil-moisture threshold, etc.) — replace this task's body with
// whatever decides when the relay should switch.
void demoCommandSourceTask(void *parameter) {
  while (true) {
    RelayCommand cmd = RELAY_CMD_ON;
    xQueueSend(relayQueue, &cmd, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(2000));

    cmd = RELAY_CMD_OFF;
    xQueueSend(relayQueue, &cmd, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  relayQueue = xQueueCreate(5, sizeof(RelayCommand));
  xTaskCreate(relayControlTask, "Relay Control", 2048, NULL, 2, NULL);
  xTaskCreate(demoCommandSourceTask, "Demo Command Source", 2048, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}