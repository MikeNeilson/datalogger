#include "system.h"
#include <cstdlib>
#include <thread>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include "gpio_cxx.hpp"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "modules.h"
#include "config.hpp"
#include "db.hpp"

using namespace std;
using namespace idf;

LoggerSystem::LoggerSystem(){ 
    
}

void LoggerSystem::init() {
    this->init_nvs();
    this->initialize_spiffs("data0");
    this->initialize_card();
    

    try {
        the_config = make_unique<Config>("/sdcard/config.db");      
    } catch (const db_exception &ex) {
        ESP_LOGE(TAG,"Config database not initialized: %s",ex.what());
    }
    
}

void LoggerSystem::init_wifi() {
    this->init_wifi();
    //auto &ssid = _conf.get<string>(prop);
}


void LoggerSystem::init_nvs() {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
 
}

void LoggerSystem::initialize_spiffs(const std::string &partition) {
ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = partition.c_str(),
      .max_files = 5,
      .format_if_mount_failed = false
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

void LoggerSystem::initialize_card() {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = HSPI_HOST;
    host.max_freq_khz = 5000;

    spi_bus_config_t bus_cfg =
        {
            .mosi_io_num = LoggerSystem::MOSI,
            .miso_io_num = LoggerSystem::MISO,
            .sclk_io_num = LoggerSystem::CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4000};

    ESP_LOGI(TAG, "Initializing spi bus");
    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &bus_cfg, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize the spi bus (%s). ", esp_err_to_name(ret));
        return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = LoggerSystem::CS;
    slot_config.host_id = HSPI_HOST;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 4 * 1024};

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t *card;
    ESP_LOGI(TAG, "Mounting SD card");
    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set format_if_mount_failed = true.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }
    has_card = true;
    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}



void LoggerSystem::run() {
    while (true)
    {
        printf("System alive\n");
        this_thread::sleep_for(std::chrono::seconds(20));
    }
}