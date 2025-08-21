#pragma once

#include "webserv.h"
#include "client.hpp"


#define CGI_BUFSIZE 4096


class CgiHandler {
public:
    CgiHandler(Request& request, sockaddr_in& clientAddr);
    ~CgiHandler();

    std::string executeCgi(Request& request);
    std::map<std::string, std::string> _env;


    void _initEnv(Request& request, sockaddr_in& clientAddr);
    char **_getEnvAsCstrArray() const;
    
};
bool check_cgi_executable(Request& request);
std::string parseCgiOutput(const std::string& rawOutput, Request& request);

std::string to_string(size_t n);

