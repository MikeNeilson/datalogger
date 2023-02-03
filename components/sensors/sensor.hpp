#pragma once
#include <vector>
#include <string>

namespace dl {

class measurement {
    private:
        std::string name;
        double value;
    public:
        measurement(const std::string& name, double value) : name(name), value(value) {}
        measurement(const measurement &other) {
            this->name = other.name;
            this->value = other.value;
        }
        measurement(measurement &&other) {
            this->name = std::move(other.name);
            this->value = other.value;
        }

        measurement& operator =(const measurement& other) {
            this->name = other.name;
            this->value = other.value;
            return *this;
        }

        measurement& operator =(const measurement&& other) {
            this->name = std::move(other.name);
            this->value = other.value;
            return *this;
        }

        const std::string &get_name() {
            return name;
        }

        const double& get_value() {
            return value;
        }
};

/**
 * @brief interface that all sensors need to implement
 * 
 */
class sensor {
    public:
        virtual std::vector<measurement> measure() = 0;
};

};

