#include "db.hpp"
#include <string>
#include "sqlite3.h"
#include <esp_log.h>



database::database(const std::string &db_file) {
    int result = sqlite3_open(db_file.c_str(),&this->db);
    if( result != SQLITE_OK ){
        throw db_exception(sqlite3_errmsg(this->db));
    }
    const char *db_name = sqlite3_db_filename(db,nullptr);
}

database::~database() {
    int result = sqlite3_close(this->db);
    if (result != SQLITE_OK) {
        ESP_LOGE("DB","error closing db %s",sqlite3_errmsg(this->db));
    }
}

database::operator sqlite3*() {
    return this->db;
}
