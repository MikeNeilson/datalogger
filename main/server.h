#ifndef DL_SERVER_H__
#define DL_SERVER_H__

#include "esp_http_server.h"
#include "system.h"

class Server {
    private:
        httpd_handle_t server;
        LoggerSystem *logger = nullptr;

    public:
        Server();
        ~Server();
        void init(LoggerSystem *logger);
};

#endif