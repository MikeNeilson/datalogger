#include "config.hpp"
#include <exception>
#include <fstream>
#include <sstream>
#include "sqlite3.h"
#include <esp_log.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

using namespace std;

Config::Config(const std::string &database) {
    
    int result = sqlite3_initialize();
    
    if( result != SQLITE_OK ) {
        std::stringstream ss;
        ss << "Unable to initialize SQLite. Error: " << result << std::endl;
        throw db_exception(ss.str().c_str());
    }

    result = sqlite3_open(database.c_str(), &db);
    if( result != SQLITE_OK ){
        throw db_exception(sqlite3_errmsg(db));
    }

    this->init_db();
    this->init_queries();
}

Config::~Config() {
    if( db != nullptr ) {
        xSemaphoreTake(mutex,portTICK_PERIOD_MS*1);
        sqlite3_close(db);
    }
}
    
void Config::init_queries() {
    int result = sqlite3_prepare_v2(this->db, "select value from config where key = ?",-1,&get_prop,nullptr);
    if( result != SQLITE_OK ){
        throw db_exception(sqlite3_errmsg(db));
    }
    result = sqlite3_prepare_v2(this->db, "insert into config(key,value) values(?,?) on conflict(key) do update set key =excluded.key",-1,&get_prop,nullptr);
    if( result != SQLITE_OK ){
        throw db_exception(sqlite3_errmsg(db));
    }
}

void Config::init_db() {
    std::ifstream init("/spiffs/dbinit.sql");
    if( !init.is_open() ){
        ESP_LOGE(TAG,"failed to open db initialization file");
        return;
    }
    std::stringstream ss;
    ESP_LOGI(TAG,"Loading sql file");
    ss << init.rdbuf();
    ESP_LOGI(TAG,"Running init");
    int sqlRes = sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, &err_msg);
    if (sqlRes != SQLITE_OK)
    {
        printf("SQL error: %s\n",err_msg);
        sqlite3_free(err_msg);
        return;
    }
}    


template<>
std::string Config::get_property_value<std::string>(const std::string& key) {
    int result = sqlite3_bind_text(get_prop,1,key.c_str(),key.size(),nullptr);
    if( result != SQLITE_OK ){
        throw db_exception(sqlite3_errmsg(db));
    }

    result = sqlite3_step(get_prop);
    if( result != SQLITE_OK ) {
        throw db_exception(sqlite3_errmsg(db));
    }
    
    return std::string(reinterpret_cast<const char*>(sqlite3_column_text(get_prop,1)));

}


template<>
void Config::set_property_value<std::string>(const string& key, const string& value) {

}