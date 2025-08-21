#pragma once

#include "webserv.h"

class Response{
    std::map<int, std::string>          _cMessage;
    // std::map<std::string, std::string>  _MimeTypes;
    std::map<int, std::string>          _CodeMessage();
    // std::map<std::string, std::string>  _initMimeTypes();
    std::string                         _MimeTypes(const std::string &Url);
    std::string                         _extratexcFilename(std::string filename);
    std::string                         _extratFilename(std::string filename);
public:
    Response();
    ~Response();

    //
    std::string                         getCodeMessage(int code);
    std::string                         ErrorResponse(int code);
    std::string                         getResponse(std::string path);
    /// 
    std::string                         getHeaderResponse(std::string Mimetype, int size, int code);
    std::string                         getRedirectResponse(const std::string& newUrl, int code);
    std::string                         Connectionstatus(std::string con);
    long long                              fileSize(std::string filePath);
    
    std::string getUploadFilename(std::string& UriPath, std::string filename, std::string rd);
};
