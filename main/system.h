#ifndef DL_SYSTEM_H__
#define DL_SYSTEM_H__

#include <string>
#include "driver/gpio.h"

class LoggerSystem {
    private:
        const gpio_num_t MISO = GPIO_NUM_19;
        const gpio_num_t MOSI = GPIO_NUM_23;
        const gpio_num_t CLK = GPIO_NUM_18;
        const gpio_num_t CS = GPIO_NUM_5;


        void initialize_card();
        void initialize_spiffs(const std::string &partition);
        void init_nvs();
        void init_wifi();

    public:
        LoggerSystem();
        void run();
        void init();

};


#endif