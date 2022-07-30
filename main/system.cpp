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

using namespace std;
using namespace idf;

LoggerSystem::LoggerSystem(){ 
    
}

void LoggerSystem::init() {
    this->init_nvs();
    this->initialize_spiffs("data0");
    this->initialize_card();
    this->init_wifi();
    
    the_config = make_unique<Config>("/sdcard/test.db");
    const char *prop = "ssid";
    auto& _conf = this->config();
    auto ssid = _conf.get<string>(prop);
    cout << "got SSID = " << ssid.value() << " from config" << endl;;
    //ESP_LOGI(TAG, ssid.value().c_str());
}

void LoggerSystem::init_wifi() {

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
        .format_if_mount_failed = true,
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

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

void LoggerSystem::load_config() {
        sqlite3 *db;
        char *zErrMsg = nullptr;
        sqlite3_initialize();
        int res = sqlite3_open("/sdcard/test.db", &db);        
        if (res != 0)
        {
            ESP_LOGE(TAG, "failed to open datbase.");
            throw std::exception("unable to open or create configuration database");
        }        
        ESP_LOGI(TAG, "config database opened, checking version level");
        

        
        ESP_LOGI(TAG,"loading init file");
        ifstream init("/spiffs/dbinit.sql");
        if( !init.is_open() ){
            ESP_LOGE(TAG,"failed to open db initialization file");
            return;
        }
        stringstream ss;
        ESP_LOGI(TAG,"Loading sql file");
        ss << init.rdbuf();
        ESP_LOGI(TAG,"Running init");
        int sqlRes = sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, &zErrMsg);
        if (sqlRes != SQLITE_OK)
        {
            printf("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }
        
        sqlRes = sqlite3_exec(db, "insert into config(key,value,type) values ('ssid','mmnet','text');", nullptr, nullptr, &zErrMsg);
        if (sqlRes != SQLITE_OK)
        {
            printf("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }
        sqlite3_stmt *stmt;
        std::string query = "select * from config where key = ?";
        sqlite3_prepare_v2(db, query.c_str(), query.size(), &stmt, NULL);
        sqlite3_bind_text(stmt, 1, "ssid", 4, NULL);
        sqlRes = sqlite3_step(stmt);
        if (sqlRes == SQLITE_ROW)
        {
            const unsigned char *ssid = sqlite3_column_text(stmt, 1);
            printf("SSID to use is %s\n", ssid);
            
        }
        else if (sqlRes == SQLITE_ERROR)
        {
            printf("Error getting ssid %s", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);

        }
}


void LoggerSystem::run() {
    while (true)
    {
        printf("System alive\n");
        this_thread::sleep_for(std::chrono::seconds(20));
    }
}