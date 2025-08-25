#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stdbool.h>

static const char *TAG = "STORAGE";
static nvs_handle_t storage_nvs_handle = 0;
static bool nvs_ready = false;

void storage_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS: sem páginas livres/versão nova -> apagando e reiniciando NVS");
        esp_err_t e = nvs_flash_erase();    
        if (e != ESP_OK) {
            ESP_LOGE(TAG, "Falha ao apagar NVS: %s", esp_err_to_name(e));
            return;
        }
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar NVS: %s", esp_err_to_name(ret));
        return;
    }

    // CORREÇÃO: usar storage_nvs_handle em vez de nvs_handle
    ret = nvs_open("storage", NVS_READWRITE, &storage_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao abrir namespace 'storage': %s", esp_err_to_name(ret));
        nvs_ready = false;
        return;
    }
    nvs_ready = true;
    ESP_LOGI(TAG, "NVS inicializado");
}

void storage_deinit(void) {
    if (nvs_ready) {
        // CORREÇÃO: usar storage_nvs_handle em vez de nvs_handle
        nvs_close(storage_nvs_handle);
        nvs_ready = false;
        storage_nvs_handle = 0;
        ESP_LOGI(TAG, "NVS fechado");
    }
}

float storage_load_hidrometer(void) {
    if (!nvs_ready) {
        ESP_LOGW(TAG, "NVS não inicializado, retornando 0.0");
        return 0.0f;
    }

    size_t size = sizeof(float);
    float liters = 0.0f;
    // CORREÇÃO: usar storage_nvs_handle em vez de nvs_handle
    esp_err_t err = nvs_get_blob(storage_nvs_handle, "hidrometro", &liters, &size);
    if (err == ESP_OK) {
        if (size != sizeof(float)) {
            ESP_LOGE(TAG, "Tamanho do blob inesperado: %u (esperado %u), iniciando em 0", (unsigned)size, (unsigned)sizeof(float));
            liters = 0.0f;
        } else {
            ESP_LOGI(TAG, "Hidrômetro recuperado: %.3f L", liters);
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Nenhum valor salvo, iniciando em 0");
        liters = 0.0f;
    } else {
        ESP_LOGE(TAG, "Erro ao ler hidrometro: %s", esp_err_to_name(err));
        liters = 0.0f;
    }
    return liters;
}

void storage_save_hidrometer(float liters) {
    if (!nvs_ready) {
        ESP_LOGW(TAG, "NVS não inicializado, não foi possível salvar");
        return;
    }

    // CORREÇÃO: usar storage_nvs_handle em vez de nvs_handle
    esp_err_t err = nvs_set_blob(storage_nvs_handle, "hidrometro", &liters, sizeof(liters));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao setar hidrometro: %s", esp_err_to_name(err));
        return;
    }
    // CORREÇÃO: usar storage_nvs_handle em vez de nvs_handle
    err = nvs_commit(storage_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao commitar NVS: %s", esp_err_to_name(err));
    }
}

void storage_reset_hidrometer(void) {
    if (!nvs_ready) {
        ESP_LOGW(TAG, "NVS não inicializado, não foi possível resetar");
        return;
    }

    float zero = 0.0f;
    // CORREÇÃO: usar storage_nvs_handle em vez de nvs_handle
    esp_err_t err = nvs_set_blob(storage_nvs_handle, "hidrometro", &zero, sizeof(zero));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao resetar hidrometro: %s", esp_err_to_name(err));
        return;
    }
    // CORREÇÃO: usar storage_nvs_handle em vez de nvs_handle
    err = nvs_commit(storage_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao commitar reset NVS: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "Hidrômetro resetado para 0.0 L");
}