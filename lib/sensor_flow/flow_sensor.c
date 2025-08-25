#include "flow_sensor.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

#include "config.h"

static const char *TAG = "FLOW";
static volatile uint32_t pulse_count = 0;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR flow_isr_handler(void* arg) {
    // quick increment in IRAM, protected by spinlock
    portENTER_CRITICAL_ISR(&mux);
    pulse_count++;
    portEXIT_CRITICAL_ISR(&mux);
}

void flow_sensor_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << FLOW_SENSOR_GPIO),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf);

    // install ISR service (safe to call multiple times)
    gpio_install_isr_service(0);
    gpio_isr_handler_add(FLOW_SENSOR_GPIO, flow_isr_handler, NULL);

    ESP_LOGI(TAG, "Flow sensor init on GPIO %d (K=%.2f)", FLOW_SENSOR_GPIO, FLOW_K_FACTOR);
}

float flow_sensor_get_increment(float interval_seconds) {
    uint32_t pulses = 0;
    // atomically grab and clear pulse_count
    portENTER_CRITICAL(&mux);
    pulses = pulse_count;
    pulse_count = 0;
    portEXIT_CRITICAL(&mux);

    // liters = pulses / (K * 60)
    if (interval_seconds <= 0.0f) return 0.0f;
    float liters = (float)pulses / (FLOW_K_FACTOR * 60.0f);
    ESP_LOGD(TAG, "Pulses=%u interval=%.3fs liters=%.6f", (unsigned)pulses, interval_seconds, liters);
    return liters;
}
