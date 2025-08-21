#pragma once

#include "webserv.h"

class Request{
    std::string                         _method;
    std::string                         _uriPath;
    std::string                         _httpV;
    std::string                         _boday;
    bool                                _isvalid;
    std::string                         _Query;
    size_t                              _contentLength;
    std::string                         _path;
    std::string                         _cgi_pass;
    int                                 _cgi_code;
    
    public:
	Request(std::string& reqMsg);
	~Request();
    
    void _parseRequestLine(std::string& reqMsg);
    void _parseHeaderFields(std::istringstream& RqHeaders);
    LocationConfig loc_config;
    std::map<std::string, std::string>  _headers;

    //
    bool isValidHeaders() const;// {}
    std::string getHeadr(std::string& key);
    // template <typename T> std::string _toString(T value);
    //
    std::string getMethod(){ return this->_method;}
    int getCgiCode() const { return this->_cgi_code; }
    void setCgiCode(int code) { this->_cgi_code = code; }
    std::string getUri(){return this->_uriPath;}
    std::string getQuery(){return this->_Query;}
    std::string getHttpVersion(){return this->_httpV;}
    std::string getBody(){return this->_boday;}
    std::string getPath() const { return this->_path; }
    std::string getCgiPass() const { return this->_cgi_pass; }    
    std::string getListContent() const;
    void setCgiPass(const std::string& cgiPass) { this->_cgi_pass = cgiPass; }
    void setPath(const std::string& path) { this->_path = path; }
    std::map<std::string, std::string> getHeaders() const { return this->_headers; }
    bool findBestLocation(const std::string& requestPath, const ServerConfig& serverConfig);
    void getFullPath(const std::string &urlPath, LocationConfig &locationConfig);
    std::string createRedirectResponse(const std::string& newUrl, int statusCode);

};
