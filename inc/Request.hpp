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
    long long                           _contentLength;
    std::string                         _cgi_pass;
    std::string                         _path;
    void                                _parseRequestLine(std::string& reqMsg);
    void                                _parseHeaderFields(std::istringstream& RqHeaders);
public:
	Request(std::string& reqMsg);
	~Request();
    //
    std::map<std::string, std::string>  getHeaders();//{return this->_headers;}
    std::string                         getQuery();//{return this->_Query;}
    std::string                         getBody();//{return this->_boday;}
    std::string                         getCgipass();//{return this->_cgi_pass;}
    std::string                         getHeadr(std::string key);
    std::string                         getMethod();//{ return this->_method;}
    std::string                         getUri();//{return this->_uriPath;}
    std::string                         getDirContent();
    std::string                         getPath();//{return this->_path;}
    long long                           getcontentLen();//{return this->_contentLength;}
    //
    void                                setHeadr(std::string key, std::string value);
    bool                                isValidHeaders() const;
    LocationConfig                      loc_config;

    //
    
    //
    bool                                findBestLocation(const std::string& requestPath, const ServerConfig& serverConfig);
    void                                getFullPath(const std::string &urlPath, LocationConfig &locationConfig);
    std::vector<FormPart>               MultipartBody(const std::string& body, const std::string& conType);
    FormPart                            BoundryBody(const std::string& part);
    std::string                         ExtractBoundry(const std::string& str);
     std::map<std::string, std::string> _FormUrlDec(const std::string& body);
};

