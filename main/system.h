#ifndef DL_SYSTEM_H__
#define DL_SYSTEM_H__

#include <string>
#include <memory>
#include "driver/gpio.h"
#include "config.hpp"

class LoggerSystem {
    private:
        const char *TAG = "LoggerSystem";
        const gpio_num_t MISO = GPIO_NUM_19;
        const gpio_num_t MOSI = GPIO_NUM_23;
        const gpio_num_t CLK = GPIO_NUM_18;
        const gpio_num_t CS = GPIO_NUM_5;
        
        std::unique_ptr<Config> the_config;


        void initialize_card();
        void initialize_spiffs(const std::string &partition);
        void init_nvs();
        void init_wifi();

    public:
        LoggerSystem();
        void run();
        void init();
        Config& config() { return *the_config; }
        //Config& config() { return *the_config; }

};


#endif