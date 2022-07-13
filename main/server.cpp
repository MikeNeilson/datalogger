#include "server.h"
#include "esp_err.h"
#include "esp_log.h"

extern "C" {
    
esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_send(req,"Hello!",HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_index = {
    .uri = "/uri",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
};

}

Server::Server() {
    
}

void Server::init() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    if( httpd_start(&server, &config) == ESP_OK ) {
        httpd_register_uri_handler(server,&uri_index);
    }
}

Server::~Server() { 
    if (server) {
        httpd_stop(server);
    }
}