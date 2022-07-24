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

        // load base config
        // load user config
        // init wifi
        // start ts db task
        // start cronjobs
        // start web server


        logger.init();
        wifi_init_station();
        server.init();
        // System system;
        // Database database("/sdcard/config.db");
        // Config config(db);
        // System.set_wifi(config);
        
        // start database task

        
        ESP_LOGI(TAG,"letting system run");
        logger.run();        
    }
    catch (GPIOException &e)
    {
        printf("GPIO exception occurred: %s\n", esp_err_to_name(e.error));
        printf("stopping.\n");
    }
    catch (exception &e){
        printf(e.what());
    }
}
