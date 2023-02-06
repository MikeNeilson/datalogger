#include "sensor.hpp"

namespace dl {

measurement::measurement(const std::string& name, double value) : name(name), value(value) {}

measurement::measurement(const char* name, double value) : name(name), value(value) {}

measurement::measurement(const measurement &other) {
        this->name = other.name;
        this->value = other.value;
}

measurement::measurement(measurement &&other) {
    this->name = std::move(other.name);
    this->value = other.value;
}

measurement& measurement::operator =(const measurement& other) {
    this->name = other.name;
    this->value = other.value;
    return *this;
}

measurement& measurement::operator =(measurement&& other) {
    this->name = std::move(other.name);
    this->value = other.value;
    return *this;
}

const std::string& measurement::get_name() const {
    return name;
}

double measurement::get_value() const {
    return value;
}

}

 
