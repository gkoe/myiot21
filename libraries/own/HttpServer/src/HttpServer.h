#pragma once

#include <esp_http_server.h>

class HttpServerClass
{
    public:
        void init();
        
    private:
    httpd_handle_t startWebserver();
};
extern HttpServerClass HttpServer;
