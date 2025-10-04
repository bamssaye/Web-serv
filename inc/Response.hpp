#pragma once

#include "webserv.h"

class Response{
    std::map<int, std::string>          _cMessage;
    std::map<int, std::string>          _CodeMessage();
    
    
public:
    Response();
    ~Response();

    //
    std::string                         getCodeMessage(int code);
    std::map<int, std::string>          getCodeMessageMap();
    std::string                         getCodeMessageHtml(int code);
    std::string                         ErrorResponse(int code, std::map<int, std::string> error_pages);
    std::string                         getResponse(std::string path, std::map<int, std::string> error_pages = std::map<int, std::string>());
    /// 
    std::string                         getHeaderResponse(std::string Mimetype, int size, int code);
    std::string                         getRedirectResponse(const std::string& newUrl, int code);
    std::string                         Connectionstatus(std::string con);
    std::string                         _MimeTypes(const std::string &Url);
};
