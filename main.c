#include "http.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h"

// ThingsBoard Configuration
#define THINGSBOARD_TOKEN "jyFG7l5unibxMMpjdGjz"
#define THINGSBOARD_URL "http://demo.thingsboard.io/api/v1/" THINGSBOARD_TOKEN "/telemetry"

#define LED1 GPIO_NUM_2
#define SENSOR_TYPE DHT_TYPE_DHT11
#define GPIO_DATA GPIO_NUM_4
static const char *TAG_HTTP = "HTTP_CLIENT";
static const char *TAG_DHT = "DHT";

void dht_thingsboard_task(void *params) {
    float Temp, Humidity;
    while (1) {
        // // Read data from DHT sensor
        // if (dht_read_float_data(SENSOR_TYPE, GPIO_DATA, &Humidity, &Temp) == ESP_OK) {
        //     ESP_LOGI(TAG_DHT, "Humidity: %.1f%%, Temp: %.1fC", Humidity, Temp);

        //     // Create JSON payload
        //     cJSON *root = cJSON_CreateObject();
        //     cJSON_AddNumberToObject(root, "temperature", Temp);
        //     cJSON_AddNumberToObject(root, "humidity", Humidity);
        //     const char *post_data = cJSON_Print(root);

        //     // Send data to ThingsBoard
        //     if (http_client_post_req((char *)post_data, THINGSBOARD_URL) == ESP_OK) {
        //         ESP_LOGI(TAG_HTTP, "Data successfully sent to ThingsBoard");
        //     } else {
        //         ESP_LOGE(TAG_HTTP, "Failed to send data to ThingsBoard");
        //     }

        //     // Clean up
        //     cJSON_Delete(root);
        //     free((void *)post_data);
        // } else {
        //     ESP_LOGE(TAG_DHT, "Could not read data from DHT sensor");
        // }
            // Read data from DHT sensor
        int dht_result = dht_read_float_data(SENSOR_TYPE, GPIO_DATA, &Humidity, &Temp);
        if (dht_result == 0) { // Check if DHT reading is successful (0 = success)
            ESP_LOGI(TAG_DHT, "Humidity: %.1f%%, Temp: %.1fC", Humidity, Temp);

            // Create JSON payload
            cJSON *root = cJSON_CreateObject();
            cJSON_AddNumberToObject(root, "temperature", Temp);
            cJSON_AddNumberToObject(root, "humidity", Humidity);
            const char *post_data = cJSON_Print(root);

            // Send data to ThingsBoard
            int http_result = http_client_post_req((char *)post_data, THINGSBOARD_URL);
            if (http_result == 0) { // Check if HTTP post is successful (0 = success)
                ESP_LOGI(TAG_HTTP, "Data successfully sent to ThingsBoard");
            } else {
                ESP_LOGE(TAG_HTTP, "Failed to send data to ThingsBoard");
            }

            // Clean up
            cJSON_Delete(root);
            free((void *)post_data);
        } else {
            ESP_LOGE(TAG_DHT, "Could not read data from DHT sensor");
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay 2 seconds
    }

    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_LOGI("APP_MAIN", "Starting application...");
    wifi_init(); // Initialize WiFi

    // Create a task to send DHT sensor data to ThingsBoard
    xTaskCreate(dht_thingsboard_task, "dht_thingsboard_task", 4096, NULL, 5, NULL);
}
