#pragma once
#include "sqlite3.h"
#include <string>
#include <optional>
#include "db.hpp"

class query;

class result {
    private:
        query &q;

        result(query &q) : q(q) {};
        
        
    public:
        result(result &&other) : q(other.q) {};
        result(const result &other) = delete;
        ~result() {}


        bool next();
        std::optional<std::string> get_string(int column);
        friend class query;
};


class query {
    private:
        sqlite3_stmt *stmt;
        sqlite3 *db;

    public:
        query(sqlite3 *db, const std::string& query);
        query(sqlite3 *db, const char* query);
        ~query();

        void bind_string(int column, const std::string& value);
        result execute();
        void executeUpdate();
        std::optional<std::string> get_string(int column);

    friend class result;
};
