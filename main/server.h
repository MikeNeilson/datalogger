#ifndef DL_SERVER_H__
#define DL_SERVER_H__

#include "esp_http_server.h"

class Server {
    private:
        httpd_handle_t server;
    public:
        Server();
        ~Server();
        void init();
};

#endif