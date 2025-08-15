#pragma once

#include "webserv.h"

struct LocationConfig {
    std::string              path;
    std::string              root;
    std::string              index;
    std::string              cgi_pass;
    bool                     autoindex;
    std::vector<Methods>     allowed_methods;
    int                      return_code;
    std::string              return_url;
    std::string              upload_path;
    std::vector<std::string> cgi_extensions;

    LocationConfig();
};

struct ServerConfig {
    unsigned long                host;
    int                          port;
    std::string                  root;
    std::vector<std::string>     server_names;
    long                         client_max_body_size;
    std::map<int, std::string>   error_pages;
    // std::pair<std::vector<int>, std::string> error_pages;
    std::vector<LocationConfig>  locations;

    ServerConfig();
};

class Parser {
public:
    Parser(const std::string& filename);
    ~Parser();

    void parse();
    const std::vector<ServerConfig>& getServers() const;
    void displayConfigs();

protected:
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

