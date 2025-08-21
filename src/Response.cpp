#include "../inc/Response.hpp"

Response::Response(){
    this->_cMessage = _CodeMessage();
}
Response::~Response(){}

std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
std::string Response::getCodeMessage(int code){
    std::map<int, std::string>::const_iterator it = _cMessage.find(code);
    if (it != _cMessage.end()) {
        std::ostringstream html;
            std::ifstream file("html/error/default.html");
            if (file.is_open()) {
                std::ostringstream buffer;
                buffer << file.rdbuf();
                std::string htmlContent = buffer.str();
                file.close();

                size_t pos;
                if((pos = htmlContent.find("codes")) != std::string::npos)
                    htmlContent.replace(pos, 5, to_string(code));
                if ((pos = htmlContent.find("errors")) != std::string::npos)
                    htmlContent.replace(pos, 6, it->second);
                html << htmlContent;
            } else {
                html << "<h1>" << code << " " << it->second << "</h1>";
            }
            return html.str();
    }
    return "Unknown Error";
}
std::string Response::ErrorResponse(int code, std::map<int, std::string> error_pages) {
        std::string body;
        std::map<int, std::string>::const_iterator it = _cMessage.find(code);

        std::ostringstream res;
        if (error_pages.find(code) != error_pages.end()) {
            std::ifstream file(error_pages.at(code).c_str());
            if (!file.is_open()) {
                body = this->getCodeMessage(code);
            } else {
                std::ostringstream buffer;
                buffer << file.rdbuf();
                body = buffer.str();
                file.close();
            }
        } else {
            body = this->getCodeMessage(code);
        }
        res << "HTTP/1.0 " << code << " " << it->second << "\r\n";
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