#pragma once
#include <exception>
#include <string>
#include "sqlite3.h"

class db_exception : protected std::exception {
    std::string msg;
    public:
        db_exception(std::string the_msg ) : msg(the_msg) {

        }

        const char* what() const noexcept override {
            return msg.c_str();
        }
};

class database {
    private:
        sqlite3* db;
    public:
        database(const std::string &db_file);
        ~database();
        operator sqlite3*();
};
