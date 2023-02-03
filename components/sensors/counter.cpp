#include "counter.hpp"

namespace dl {
using std::string;
using std::vector;

Counter::Counter(int pin, bool count_on_high, pcnt_unit_t unit, pcnt_channel_t channel) {
    this->unit = unit;
    this->unit_config= pcnt_config_t{
            // Set PCNT input signal and control GPIOs
            .pulse_gpio_num = pin,
            .ctrl_gpio_num = -1,
            .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
            .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
            // What to do on the positive / negative edge of pulse input?
            .pos_mode = count_on_high ? PCNT_COUNT_INC : PCNT_COUNT_DIS,   // Count up on the positive edge
            .neg_mode = count_on_high ? PCNT_COUNT_DIS : PCNT_COUNT_INC,   // Keep the counter value on the negative edge
            // What to do when control input is low or high?
            // Set the maximum and minimum limit values to watch
            .counter_h_lim = 0,
            .counter_l_lim = 0,
            .unit = unit,
            .channel = channel,
    };

    pcnt_unit_config(&unit_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(unit, 1000);
    pcnt_filter_enable(unit);
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);
    pcnt_counter_resume(unit);
}

std::vector<measurement> Counter::measure() {
    int16_t count;
    pcnt_counter_pause(unit);
    pcnt_get_counter_value(unit,&count);
    pcnt_counter_clear(unit);
    pcnt_counter_resume(unit);
    measurements[0] = {"count",static_cast<double>(count)};
    return measurements;
}

};