#include "query.hpp"
#include "esp_log.h"


bool result::next() {
    int result = sqlite3_step(q.stmt);
    if (result == SQLITE_DONE) {
        ESP_LOGE("result","done");
        return false;
    } else if (result == SQLITE_ROW) {
        ESP_LOGE("result","row");
        return true;
    } else {        
        ESP_LOGE("result","Unable to run/step query: %s", sqlite3_errmsg(q.db));
        throw new db_exception("Unable to run/step query");
    }    
}

std::optional<std::string> result::get_string(int column) {
    ESP_LOGI("result","getting string from query");
    return q.get_string(column);
}

query::query(sqlite3* db, const std::string &query) {
    this->db = db;
    int result = sqlite3_prepare_v2(db,query.c_str(),-1,&this->stmt,nullptr);
    if( result != SQLITE_OK ) {
        ESP_LOGE("query","Unable to prepare query: %s", sqlite3_errmsg(db));
        throw new db_exception("Unable to prepare query ");
    }
}

query::query(sqlite3* db, const char* query) {
    this->db = db;
    int result = sqlite3_prepare_v2(db,query,-1,&this->stmt,nullptr);
    if( result != SQLITE_OK ) {
        ESP_LOGE("query","Unable to prepare query: %s, %s", query,sqlite3_errmsg(db));
        throw new db_exception("Unable to prepare query");
    }
}


query::~query() {
    int result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK ) {
        ESP_LOGE("query", "Unable to destroy query %s",sqlite3_errmsg(db));
    }
    
}

void query::bind_string(int column, const std::string& value) {
    int result = sqlite3_bind_text(stmt,column,value.c_str(),value.size(),nullptr);
    if( result != SQLITE_OK ){
        throw db_exception(sqlite3_errmsg(db));
    }
}

result query::execute() {
    return result(*this);    
}

void query::executeUpdate() {
    int result = sqlite3_step(stmt);
    if( result != SQLITE_OK ){
        throw db_exception(sqlite3_errmsg(db));
    }
}

std::optional<std::string> query::get_string(int column) {
    ESP_LOGI("query","getting column text");
    int type = sqlite3_column_type(stmt,column);
    if (type == SQLITE_TEXT) {
        auto *text_raw = sqlite3_column_text(stmt,column);
        std::string text(reinterpret_cast<const char*>(text_raw));
        return text;
    } else if( type == SQLITE_NULL) {
        return std::nullopt;
    } else {
        throw new db_exception("not a string value");
    }
    
}