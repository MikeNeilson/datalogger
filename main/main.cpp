/* Blink C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
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
#include "driver/pcnt.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include "gpio_cxx.hpp"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "counter.hpp"
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


LoggerSystem logger;        
Server server;
//TSDB timedb;

extern "C" void app_main(void)
{
    const char *TAG = "LOGGER";
    try
    {
        // baseline hardware
        //logger.init();
        // database task
        // config task
        // setup wifi, etc
        //wifi_init_station();
        // web server
        //server.init();
        // measurement tasks (cron?)


     
        TickType_t last_wake_time;
        BaseType_t xWasDelayed;
        last_wake_time = xTaskGetTickCount();
        const uint16_t freqS = 10;
        const TickType_t freqTicks = pdMS_TO_TICKS(freqS*1000);
        dl::Counter counter(GPIO_NUM_12,false);
        for(;;)
        {
            xWasDelayed = xTaskDelayUntil( &last_wake_time, freqTicks );
            TickType_t current_time = xTaskGetTickCount();
            
            if( xWasDelayed == pdTRUE ) {
                ESP_LOGI("RAIN","was delayed");
            }
            ESP_LOGI("RAIN","Getting pulse count.");
            double count = counter.measure()[0].get_value();
            TickType_t elapsed_ticks = (current_time-last_wake_time);
            float rate = static_cast<float>(count)/static_cast<float>(freqS+elapsed_ticks);

            ESP_LOGI("RAIN","\t(current_tick,last_ticks,count,elapsed_ticks,rate)->");
            ESP_LOGI("RAIN","\t %09d        ,%09d      ,%03f ,%05d         ,%f)",current_time,last_wake_time,count,elapsed_ticks,rate);
            
        }
        

        
        // System system;
        // Database database("/sdcard/config.db");
        // Config config(db);
        // System.set_wifi(config);
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
