#pragma once

#include "webserv.h"

class Response{
    std::map<int, std::string>  _cMessage;
    std::map<int, std::string>  _CodeMessage();
public:
    Response();
    ~Response();

    //
    std::string getCodeMessage(int code);
    std::string ErrorResponse(int code);
    std::string getResponse();
};

