#ifndef DL_CONFIG_H__
#define DL_CONFIG_H__
#include <exception>
#include <fstream>
#include <sstream>
#include <variant>
#include <any>
#include <map>
#include <functional>
#include "sqlite3.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

template<typename T>
class property;

template<typename T>
class property {

    typedef std::function<T(const std::string&)> get_prop_func;
    typedef std::function<void(const std::string&, const T&)> set_prop_func;
    
    std::string the_key;
    T the_value;
    get_prop_func get_prop;
    set_prop_func set_prop;

    public:
        property(){}        
        property(const std::string& key, get_prop_func get_func, set_prop_func set_func){
            this->the_key = key;
            this->get_prop = get_func;
            this->set_prop = set_func;
        }
        /*property(const std::string& key, const T &value) {
            this->key = key;
            this->value = value;
        }*/
        property(const property<T>& other) {
            this->the_key = other.the_key;
            this->the_value = other.the_value;
            this->get_prop = other.get_prop;
            this->set_prop = other.set_prop;
        }

        property(property<T>&& other ) {
            this->the_key = std::move(other.the_key);
            this->the_value= std::move(other.the_value);
            this->get_prop = std::move(other.get_prop);
            this->set_prop = std::move(other.set_prop);
        }

        property<T>& operator=(T value) {
            this->the_value = value;
            set_prop(the_key,the_value);
            return *this;
        }

        property<T>& operator=(const property<T>& rval) {
            this->the_key = rval.the_key;
            this->the_value = rval.the_value;
            set_prop(the_key,the_value);
            return *this;
        }

        property<T>& operator=(property<T>&& rval) {
            this->the_key = std::move(rval.the_key);
            this->the_value = std::move(rval.the_value);
            this->get_prop = std::move(rval.get_prop);
            this->set_prop = std::move(rval.set_prop);
            return *this;
        }

        const T& value() { 
            this->the_value = get_prop(this->the_key);
            return the_value; 
        }
        const std::string& key() { return the_key; }
};

class db_exception : protected std::exception {
    std::string msg;
    public:
        db_exception(std::string the_msg ) : msg(the_msg) {

        }

        const char* what() const noexcept override {
            return msg.c_str();
        }
};

class Config {

    typedef std::variant<property<int>,property<double>,property<std::string>> prop_types;

    const char *TAG = "Config";
    sqlite3 *db = nullptr;
    char *err_msg;
    SemaphoreHandle_t mutex;
    sqlite3_stmt *get_prop = nullptr;
    sqlite3_stmt *set_prop = nullptr;

    std::map<std::string,prop_types> properties;

    public:
        Config(const std::string &database);
        ~Config();

        template<typename T>
        property<T>& get(const std::string& key ){
            auto pair = properties.find(key);
            if( pair == properties.end() ) {                
                auto prop = property<T>(
                                key,
                                [this](const std::string &key){
                                    return this->get_property_value<T>(key);
                                },
                                [this](const std::string &key, const T& value){
                                    this->set_property_value<T>(key,value);
                                }
                            );
                properties.insert(std::pair<std::string,property<T>>(key,prop));
            }
            return std::get<property<T>>(this->properties[key]);
        }

        template<typename T>
        property<T>& get(std::string&& key) {
            std::string tmp = std::move(key);
            return this->get<T>(tmp);
        }


    private:
        void init_db();
        void init_queries();

        template<typename T>
        T get_property_value(const std::string& key);

        template<typename T>
        void set_property_value(const std::string& key, const T& value);

};




#endif