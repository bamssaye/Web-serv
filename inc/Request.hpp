#pragma once

#include "webserv.h"

class Request{
    std::string                         _method;
    std::string                         _uriPath;
    std::string                         _httpV;
    std::map<std::string, std::string>  _headers;
    std::string                         _boday;
    bool                                _isvalid;
    std::string                         _Query;
    int                                 _contentLength;
    std::string                         _cgi_pass;
    std::string                         _path;
    void                                _parseRequestLine(std::string& reqMsg);
    void                                _parseHeaderFields(std::istringstream& RqHeaders);

public:
	Request(std::string& reqMsg);
    Request();
	~Request();
    //
    std::map<std::string, std::string>  getHeaders();
    std::string                         getQuery();
    std::string                         &getBody();
    std::string                         getCgipass();
    std::string                         getHeadr(std::string key);
    std::string                         getMethod();
    std::string                         getUri();
    std::string                         getDirContent();
    std::string                         getPath();
    int                                 getcontentLen();
    //
    void                                setHeadr(std::string key, std::string value);
    void                                setCgipass(std::string cgi);
    bool                                isValidHeaders() const;
    LocationConfig                      loc_config;
    //
    bool                                findBestLocation(const std::string& requestPath, const ServerConfig& serverConfig);
    void                                getFullPath(const std::string &urlPath, LocationConfig &locationConfig);
    std::map<std::string, std::string>  FormUrlDec(const std::string& body);
};

