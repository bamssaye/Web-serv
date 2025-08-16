#pragma once

#include "webserv.h"

class Parser {
public:
    Parser(const std::string& filename);
    ~Parser();

    void parse();
    std::vector<ServerConfig>& getServers();
    void displayConfigs();

private:
    std::string               _filename;
    std::vector<ServerConfig> _servers;

    std::string readFile();
    std::vector<std::string> tokenize(const std::string& content);
    void parseServerDirective(const std::vector<std::string>& tokens, size_t& i, ServerConfig& config);
    void parseLocationDirective(const std::vector<std::string>& tokens, size_t& i, LocationConfig& config);
    unsigned long custom_inet_addr(const std::string& ip_str);
    bool isNumber(const std::string& s);
    void validatePort(const std::string& portStr);
    void validateHost(const std::string& host);
    void validateRoot(const std::string& path);
    void validateMethods(const std::vector<Methods>& methods);
    void validateReturnCode(int code);
    void validateErrorPageCode(int code);
    void validateAutoindex(const std::string& val);
};

