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
public:
	Request(std::string& reqMsg);
	~Request();

    bool _parseRequestLine(std::string& reqMsg);
    bool _parseHeaderFields();

};

bool Request::_parseRequestLine(std::string& RqLine){
    std::stringstream ss(RqLine);
    size_t queryPos;

    ss >> _method;
    ss >> _uriPath;
    ss >> _httpV;

    std::string Methods[] = {"GET", "POST", "DELETE"};
    if ((queryPos =_uriPath.find('?')) != std::string::npos){
        _Query = _uriPath.substr(queryPos + 1);
        _uriPath = _uriPath.substr(0, queryPos);
    }
    for(int i = 0; i < 4 ; i++){
        if (i == 3)
            return true;
        if (Methods[i] == _method)
            break;
    }
    if (_httpV != "HTTP/1.0" && _httpV != "HTTP/1.1")
        return true;
    return false;
}

bool Request::_parseHeaderFields(){
    
}