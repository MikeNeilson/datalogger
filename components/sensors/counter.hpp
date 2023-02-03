#pragma once
#include "sensor.hpp"
#include "driver/pcnt.h"


namespace dl {

class Counter : public sensor {
    private:
   /* pulse counting code*/
        pcnt_unit_t unit;
        pcnt_config_t unit_config;
        std::vector<measurement> measurements;        


    public:
        Counter(int pin, bool count_on_high = true, pcnt_unit_t unit = PCNT_UNIT_0, pcnt_channel_t channel = PCNT_CHANNEL_0 );
        virtual std::vector<measurement> measure();
};

};


