#include "config.hpp"
#include <exception>
#include <fstream>
#include <sstream>
#include <streambuf>
#include "sqlite3.h"
#include "query.hpp"
#include "db.hpp"
#include <esp_log.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

using namespace std;

Config::Config(const std::string &database) {
    
    int result = sqlite3_initialize();
    
    if( result != SQLITE_OK ) {
        std::stringstream ss;
        ss << "Unable to initialize SQLite. Error: " << result << std::endl;
        throw db_exception(ss.str().c_str());
    }
    this->init_db();
    
}

Config::~Config() {    
}

void Config::init_db() {
    using std::string;
    using std::ifstream;
    using std::stringbuf;
    database db(database_file);    
    query new_db(db,"SELECT name FROM sqlite_master WHERE type='table' AND name=?");
    new_db.bind_string(1,"schema_history");
    bool isNewDb = !new_db.execute().next();

    if (!isNewDb) {
        ESP_LOGI(TAG,"The following database files have already been applied");
        query applied(db,"select file_name from schema_history");
        auto result = applied.execute();
        while (result.next()) {
            ESP_LOGI(TAG,"next.");
            auto file_name = result.get_string(0).value_or("Unknown filename somehow?");
            ESP_LOGI(TAG,"\t%s",file_name.c_str());
        }
    }

    DIR* db_dir = opendir("/spiffs/db");
    dirent *file = nullptr;
    while( (file = readdir(db_dir)) != nullptr ) {
        std::string file_name = std::string("/spiffs/db/") + file->d_name;
        ESP_LOGI(TAG,"Checking for file %s",file_name.c_str());
        query q(db,"select file_name from schema_history where file_name=?");
        q.bind_string(1,file_name);
        if (isNewDb || !q.execute().next()) {
            ESP_LOGI(TAG,"File not found in history, loading again");
            load_file(db,file_name);
            ESP_LOGI(TAG,"Now marking as saved");
            query insert_filename(db,"insert into schema_history(file_name) values(?)");
            insert_filename.bind_string(1,file_name);
            insert_filename.executeUpdate();
        }
    }
    closedir(db_dir);
    ESP_LOGI(TAG, "Database read in/verified.");
}

void Config::load_file(sqlite3* db, const std::string &file) {
    ifstream init(file); 
    if( !init.is_open() ) {
        ESP_LOGE(TAG,"failed to open db initialization file %s because %s",file.c_str(),strerror(errno));
        return;
    }
    ESP_LOGD(TAG,"Processing init file");
    while (!init.eof()) {
        string line;
        std::getline(init,line,';');
        
        if (line.empty()) {
            continue;
        }
        ESP_LOGD(TAG,"Executing: %s",line.c_str());
        int sqlRes = sqlite3_exec(db, line.c_str(), nullptr, nullptr, &err_msg);        
        if (sqlRes != SQLITE_OK)
        {
            ESP_LOGE(TAG,"SQL error: %s",err_msg);
            sqlite3_free(err_msg);
            throw db_exception(sqlite3_errmsg(db));
        }
    }
}    


template<>
std::string Config::get_property_value<std::string>(const std::string& key) {
    database db(database_file);
    query q(db,this->get_prop_query);
    q.bind_string(1,key);
    result r = q.execute();
    if( r.next() ) {
        std::string v = r.get_string(0).value_or("");
        ESP_LOGV(TAG,"Got prop value %s",v.c_str());
        return v;
    } else {
        ESP_LOGV(TAG,"prop doesn't yet exist");
        return "";
    }
}


template<>
void Config::set_property_value<std::string>(const string& key, const string& value) {
    database db(database_file);
    query q(db,this->set_prop_query);
    q.bind_string(1,key);
    q.bind_string(2,value);
    q.executeUpdate();
    ESP_LOGV(TAG,"saved property %s,%s",key.c_str(),value.c_str());
}