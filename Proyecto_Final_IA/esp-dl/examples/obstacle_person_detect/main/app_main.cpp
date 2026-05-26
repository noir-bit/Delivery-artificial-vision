#include "espdet_detect.hpp"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "dl_image_jpeg.hpp"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "obstacle_person_detect_spiffs";

static esp_err_t mount_spiffs() {
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

extern "C" void app_main(void)
{
    if (mount_spiffs() != ESP_OK) {
        return;
    }

    ESP_LOGI(TAG, "Loading ESP-DL Model...");
    ESPDetDetect *detect = new ESPDetDetect();
    ESP_LOGI(TAG, "Model loaded successfully.");

    while (true) {
        DIR* dir = opendir("/spiffs");
        if (dir == NULL) {
            ESP_LOGE(TAG, "Failed to open directory");
            return;
        }

        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".jpg") != NULL || strstr(ent->d_name, ".jpeg") != NULL) {
                char filepath[300];
                snprintf(filepath, sizeof(filepath), "/spiffs/%s", ent->d_name);
                
                ESP_LOGI(TAG, "Processing %s", filepath);

                // Read JPEG from SPIFFS
                dl::image::jpeg_img_t jpeg = dl::image::read_jpeg(filepath);
                if (jpeg.data == nullptr) {
                    ESP_LOGE(TAG, "Failed to read %s", filepath);
                    continue;
                }

                // Decode JPEG to RGB888
                dl::image::img_t img = dl::image::sw_decode_jpeg(jpeg, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
                if (img.data == nullptr) {
                    ESP_LOGE(TAG, "Failed to decode %s", filepath);
                    free(jpeg.data);
                    continue;
                }

                // Run inference
                auto &results = detect->run(img);

                // Print results in JSON format for the Python script
                printf("\n>>>JSON_START<<<\n");
                printf("{\n");
                printf("  \"image\": \"%s\",\n", ent->d_name);
                printf("  \"detections\": [\n");
                
                size_t idx = 0;
                for (auto it = results.begin(); it != results.end(); ++it, ++idx) {
                    const auto &res = *it;
                    printf("    {\"category\": %d, \"score\": %f, \"box\": [%d, %d, %d, %d]}%s\n",
                           res.category, res.score, res.box[0], res.box[1], res.box[2], res.box[3],
                           (idx == results.size() - 1) ? "" : ",");
                }
                printf("  ]\n");
                printf("}\n");
                printf(">>>JSON_END<<<\n\n");

                // Free image memory
                free(jpeg.data);
                free(img.data);

                // Short delay to allow Python script to process
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
        closedir(dir);
        
        ESP_LOGI(TAG, "Finished processing all images. Waiting 5 seconds before restarting...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    delete detect;
}
