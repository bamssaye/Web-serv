#pragma once

#include "webserv.h"

class Parser {
public:
    Parser(const std::string& filename);
    ~Parser();

    void parse();
    void displayConfigs();

    std::string               _filename;
    ServerConfig              _server;

    std::string readFile();
    std::vector<std::string> tokenize(const std::string& content);
    void parseServerDirective(const std::vector<std::string>& tokens, size_t& i, ServerConfig& config);
    void parseLocationDirective(const std::vector<std::string>& tokens, size_t& i, LocationConfig& config);
    unsigned long custom_inet_addr(const std::string& ip_str);
    bool isNumber(const std::string& s);
    void validatePort(const std::string& portStr);
    void validateHost(const std::string& host);
    void validateRoot(std::string& path);
    void validateMethods(const std::vector<std::string>& methods);
    void validateReturnCode(int code);
    void validateErrorPageCode(int code);
    void validateAutoindex(const std::string& val);
    void validateListenDirectives(const std::vector<std::pair<unsigned long, unsigned short> >& listen_sockets);
    std::string ipToString(unsigned long ip);
};
