/* Blink C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <cstdlib>
#include <thread>
#include <cstdio>
#include <cstring>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sqlite3.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include "gpio_cxx.hpp"

#include "loggerdb.hpp"

const char *TAG= "LOGGER";

using namespace idf;
using namespace std;
/*
CS  	GPIO 5
MOSI	GPIO 23
CLK	    GPIO 18
MISO	GPIO 19
*/

const gpio_num_t MISO = GPIO_NUM_19;
const gpio_num_t MOSI = GPIO_NUM_23;
const gpio_num_t CLK  = GPIO_NUM_18;
const gpio_num_t CS   = GPIO_NUM_5;

void initialize_card() {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = HSPI_HOST;
    host.max_freq_khz = 5000;

    spi_bus_config_t bus_cfg = 
    {
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .sclk_io_num = CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000
    };

    ESP_LOGI(TAG, "Initializing spi bus");
    esp_err_t ret = spi_bus_initialize(HSPI_HOST,&bus_cfg,1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize the spi bus (%s). ", esp_err_to_name(ret));
        return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CS;
    slot_config.host_id = HSPI_HOST;
    
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 4 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    ESP_LOGI(TAG,"Mounting SD card");
    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

}


extern "C" void app_main(void)
{
    /* The functions of GPIO_Output throws exceptions in case of parameter errors or if there are underlying driver
       errors. */
    try {
        initialize_card();
        // System system;
        // Database database("/sdcard/config.db");
        // Config config(db);
        // System.set_wifi(config);

        sqlite3 *db;
        sqlite3_initialize();
        int res = sqlite3_open("/sdcard/test.db",&db);
        if( res !=0 )  {
            ESP_LOGE(TAG,"failed to open datbase.");
        }
        ESP_LOGI(TAG,"database opened");
        /* This line may throw an exception if the pin number is invalid.
         * Alternatively to 4, choose another output-capable pin. */
        char *zErrMsg = nullptr;
        int sqlRes = sqlite3_exec(db,logger::dbsql::create_db.c_str(),nullptr,nullptr,&zErrMsg);
        if ( sqlRes != SQLITE_OK ) {
            printf("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        sqlRes = sqlite3_exec(db,"insert into config(key,value,type) values ('ssid','mmnet','text');",nullptr,nullptr,&zErrMsg);
        if ( sqlRes != SQLITE_OK ) {
            printf("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        sqlite3_stmt *stmt;
        std::string query = "select * from config where key = ?";
        sqlite3_prepare_v2(db,query.c_str(),query.size(),&stmt,NULL);
        sqlite3_bind_text(stmt,1,"ssid",4,NULL);
        sqlRes = sqlite3_step(stmt);
        if( sqlRes == SQLITE_ROW ) {
            const unsigned char *ssid = sqlite3_column_text(stmt,1);
            printf("SSID to use is %s\n",ssid);
            //sqlite3_free(ssid);
        } else if( sqlRes == SQLITE_ERROR ) {
            printf("Error getting ssid %s", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);

        GPIO_Output gpio(GPIONum(4));

        while (true) {
            printf("LED ON\n");
            gpio.set_high();
            this_thread::sleep_for(std::chrono::seconds(1));
            printf("LED OFF\n");
            gpio.set_low();
            this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (GPIOException &e) {
        printf("GPIO exception occurred: %s\n", esp_err_to_name(e.error));
        printf("stopping.\n");
    }
}
