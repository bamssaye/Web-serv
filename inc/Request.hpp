#pragma once

#include "webserv.h"

class Request{
    public:
    std::string                         _method;
    std::string                         _uriPath;
    std::string                         _httpV;
    std::map<std::string, std::string>  _headers;
    std::string                         _boday;
    bool                                _isvalid;
    std::string                         _Query;
    size_t                              _contentLength;
    std::string                         _path;
    std::string                         _cgi_pass;

	Request(std::string& reqMsg);
	~Request();

    void _parseRequestLine(std::string& reqMsg);
    void _parseHeaderFields(std::istringstream& RqHeaders);
    LocationConfig loc_config;

    //
    bool isValidHeaders() const;// {}
    std::string getHeadr(std::string& key);
    // template <typename T> std::string _toString(T value);
    //
    std::string getMethod(){ return this->_method;}
    std::string getUri(){return this->_uriPath;}
    std::string getListContent() const;
    bool findBestLocation(const std::string& requestPath, const ServerConfig& serverConfig);
    void getFullPath(const std::string &urlPath, LocationConfig &locationConfig);
    std::string createRedirectResponse(const std::string& newUrl, int statusCode);

};
