#include "query.hpp"
#include "esp_log.h"


bool result::next() {
    int result = sqlite3_step(q.stmt);
    return result == SQLITE_OK;
}

std::string result::get_string(int column) {
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
        ESP_LOGE("query", "Unable to finalized query %s",sqlite3_errmsg(db));
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

std::string query::get_string(int column) {
    auto *text_raw = sqlite3_column_text(stmt,column);
    std::string text(reinterpret_cast<const char*>(text_raw));
    return text;
}