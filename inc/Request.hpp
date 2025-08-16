#pragma once

#include "webserv.h"

class Request{
    std::string                         _method;
    std::string                         _uriPath;
    std::string                         _httpV;
    std::map<std::string, std::string>  _headers;
    std::string                         _boday;
    bool                                _isvalid;
public:
	Request(std::string& reqMsg);
	~Request();

    bool _parseRequestLine();
    bool _parseHeaderFields();

};

bool Request::_parseRequestLine(){
    
}