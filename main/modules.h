#ifndef __MODULES_H_
#define __MODULES_H_
#include "config.hpp"


extern "C" {
    void wifi_init(Config& config);
}

extern "C" {
    void measure_sensors(void *config);
}

#endif