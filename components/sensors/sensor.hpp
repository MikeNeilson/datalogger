#pragma once
#include <vector>
#include <string>

namespace dl {

class measurement {
    private:
        std::string name;
        double value;
    public:
        measurement(const std::string& name, double value);
        measurement(const char* name, double value);
        measurement(const measurement &other);
        measurement(measurement &&other);

        measurement& operator =(const measurement& other);
        measurement& operator =(measurement&& other);

        const std::string &get_name() const;
        double get_value() const;
};

/**
 * @brief interface that all sensors need to implement
 * 
 */
class sensor {
    public:
        virtual std::vector<measurement> measure() = 0;
        virtual ~sensor() {};
};

};

