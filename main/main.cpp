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

#include "sqlite3.h"

#include "modules.h"
#include "system.h"
#include "server.h"


using namespace idf;
using namespace std;
/*
CS  	GPIO 5
MOSI	GPIO 23
CLK	    GPIO 18
MISO	GPIO 19
*/
const char *TAG = "LOGGER";

LoggerSystem logger;        
Server server;

extern "C" void app_main(void)
{
    try
    {
        logger.init();        
        wifi_init_station();
        server.init();
        // System system;
        // Database database("/sdcard/config.db");
        // Config config(db);
        // System.set_wifi(config);

        sqlite3 *db;
        sqlite3_initialize();
        int res = sqlite3_open("/sdcard/test.db", &db);
        if (res != 0)
        {
            ESP_LOGE(TAG, "failed to open datbase.");
        }
        else 
        {
            ESP_LOGI(TAG, "database opened");
            /* This line may throw an exception if the pin number is invalid.
            * Alternatively to 4, choose another output-capable pin. */
            char *zErrMsg = nullptr;
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
        ESP_LOGI(TAG,"letting system run");
        logger.run();        
    }
    catch (GPIOException &e)
    {
        printf("GPIO exception occurred: %s\n", esp_err_to_name(e.error));
        printf("stopping.\n");
    }
}
