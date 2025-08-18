#include "../inc/Response.hpp"

Response::Response(){
    this->_cMessage = _CodeMessage();
}
Response::~Response(){}
std::string Response::getCodeMessage(int code){
    std::map<int, std::string>::const_iterator it = _cMessage.find(code);
    return (it != _cMessage.end()) ? it->second : "Unknown Error";
}
std::string Response::ErrorResponse(int code){
        std::ostringstream res;
        // std::string body = this->getHtmlContent(code);
        std::string body = this->getCodeMessage(code);
        res << "HTTP/1.0 " << code << " " << this->getCodeMessage(code) << "\r\n";
        res << "Content-Type: text/html\r\n";
        res << "Content-Length: " << body.size() << "\r\n\r\n";
        res << body;
        return res.str();
}

std::map<int, std::string> Response::_CodeMessage(){
    std::map<int, std::string> status;
    status[200] = "OK";
    status[201] = "Created";
    status[202] = "Created";
    status[204] = "No Content";

    status[301] = "Moved Permanently";
    status[302] = "Moved Temporarily";
    status[304] = "Not Modified";

    status[400] = "Bad Request";
    status[401] = "Unauthorized";
    status[403] = "Forbidden";
    status[404] = "Not Found";

    status[500] = "Internal Server Error";
    status[501] = "Not Implemented";
    status[502] = "Bad Gateway";
    status[503] = "Service Unavailable";
    return status;
}