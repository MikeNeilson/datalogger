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
        // baseline hardware
        //logger.init();
        // database task
        // config task
        // setup wifi, etc
        //wifi_init_station();
        // web server
        //server.init();
        // measurement tasks (cron?)
/*
        gpio_config_t io_conf = {};
        //disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        //set as output mode
        io_conf.mode = GPIO_MODE_INPUT;
        //bit mask of the pins that you want to set,e.g.GPIO18/19
        io_conf.pin_bit_mask = GPIO_NUM_12;
        //disable pull-down mode
        io_conf.pull_down_en = (gpio_pulldown_t)0;
        //disable pull-up mode
        io_conf.pull_up_en = (gpio_pullup_t)1;
        //configure GPIO with the given settings
        gpio_config(&io_conf);*/

        /* pulse counting code*/
        pcnt_unit_t unit = PCNT_UNIT_0;
        pcnt_config_t unit_config = {
            // Set PCNT input signal and control GPIOs
            .pulse_gpio_num = GPIO_NUM_12,
            .ctrl_gpio_num = -1,
            .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
            .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
            // What to do on the positive / negative edge of pulse input?
            .pos_mode = PCNT_COUNT_DIS,   // Count up on the positive edge
            .neg_mode = PCNT_COUNT_INC,   // Keep the counter value on the negative edge
            // What to do when control input is low or high?
            // Set the maximum and minimum limit values to watch
            .counter_h_lim = 0,
            .counter_l_lim = 0,
            .unit = unit,
            .channel = PCNT_CHANNEL_0,
        };
        pcnt_unit_config(&unit_config);

        /* Configure and enable the input filter */
        pcnt_set_filter_value(unit, 1000);
        pcnt_filter_enable(unit);
        pcnt_counter_pause(unit);
        pcnt_counter_clear(unit);
        pcnt_counter_resume(unit);

        TickType_t last_wake_time;
        BaseType_t xWasDelayed;
        last_wake_time = xTaskGetTickCount();
        const uint16_t freqS = 10;
        const TickType_t freqTicks = pdMS_TO_TICKS(freqS*1000);

        for(;;)
        {
            xWasDelayed = xTaskDelayUntil( &last_wake_time, freqTicks );
            TickType_t current_time = xTaskGetTickCount();
            
            if( xWasDelayed == pdTRUE ) {
                ESP_LOGI("RAIN","was delayed");
            }
            ESP_LOGI("RAIN","Getting pulse count.");
            int16_t count;
            TickType_t elapsed_ticks = (current_time-last_wake_time);
            float rate = static_cast<float>(count)/static_cast<float>(freqS+elapsed_ticks);
            pcnt_counter_pause(unit);
            pcnt_get_counter_value(unit,&count);
            pcnt_counter_clear(unit);
            pcnt_counter_resume(unit);
            ESP_LOGI("RAIN","\t(current_tick,last_ticks,count,elapsed_ticks,rate)->");
            ESP_LOGI("RAIN","\t %09d        ,%09d      ,%03d ,%05d         ,%f)",current_time,last_wake_time,count,elapsed_ticks,rate);
            
        }
        

        
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
