#include "sim7600.h"
#include "esp_log.h"

static const char *TAG = "SIM7600";

void sim7600_init(void) {
    ESP_LOGI(TAG, "Módulo SIM7600 inicializado (stub)");
}

void sim7600_send_data(const char *data) {
    ESP_LOGI(TAG, "Enviando via SIM7600: %s", data);
}
