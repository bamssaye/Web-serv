#include "../inc/Response.hpp"

/// 
Response::Response(){
    this->_cMessage = _CodeMessage();
}
Response::~Response(){}

///
std::map<int, std::string> Response::getCodeMessageMap(){return this->_cMessage;}
std::string Response::getCodeMessage(int code){
    std::map<int, std::string>::const_iterator it = _cMessage.find(code);
    return (it != _cMessage.end()) ? it->second : "Unknown Error";
}
std::string Response::getCodeMessageHtml(int code){
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
                    htmlContent.replace(pos, 5, _toString(code));
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
                body = this->getCodeMessageHtml(code);
            } else {
                std::ostringstream buffer;
                buffer << file.rdbuf();
                body = buffer.str();
                file.close();
            }
        } else {
            body = this->getCodeMessageHtml(code);
        }
        res << "HTTP/1.0 " << code << " " << it->second << "\r\n";
        res << "Content-Type: text/html\r\n";
        res << "Content-Length: " << body.size() << "\r\n";
        res << Connectionstatus("close");
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
    status[504] = "Gateway Timeout";
    return status;
}
// std::string Response::getTypes(const std::string &Url){

// }
std::string findExtensionByMime(const std::map<std::string, std::string>& m, const std::string& mime) {
    for (std::map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); ++it) {
        if (it->second == mime) {
            return it->first;
        }
    }
    return "-1";
}
std::string Response::_MimeTypes(const std::string &Url){
    std::map<std::string, std::string> m;
    m[".html"] = "text/html";
    m[".htm"]  = "text/html";
    m[".css"]  = "text/css";
    m[".js"]   = "application/javascript";
    m[".json"] = "application/json";
    m[".xml"]  = "application/xml";
    m[".txt"]  = "text/plain";
    m[".csv"]  = "text/csv";
    m[".png"]  = "image/png";
    m[".jpg"]  = "image/jpeg";
    m[".jpeg"] = "image/jpeg";
    m[".gif"]  = "image/gif";
    m[".bmp"]  = "image/bmp";
    m[".webp"] = "image/webp";
    m[".svg"]  = "image/svg+xml";
    m[".ico"]  = "image/x-icon";
    m[".woff"]  = "font/woff";
    m[".woff2"] = "font/woff2";
    m[".ttf"]   = "font/ttf";
    m[".otf"]   = "font/otf";
    m[".eot"]   = "application/vnd.ms-fontobject";
    m[".mp3"]   = "audio/mpeg";
    m[".wav"]   = "audio/wav";
    m[".ogg"]   = "audio/ogg";
    m[".mp4"]   = "video/mp4";
    m[".webm"]  = "video/webm";
    m[".ogv"]   = "video/ogg";
    m[".avi"]   = "video/x-msvideo";
    m[".mov"]   = "video/quicktime";
    m[".mkv"]   = "video/x-matroska";
    m[".zip"]   = "application/zip";
    m[".tar"]   = "application/x-tar";
    m[".gz"]    = "application/gzip";
    m[".bz2"]   = "application/x-bzip2";
    m[".7z"]    = "application/x-7z-compressed";
    m[".rar"]   = "application/vnd.rar"; 
    m[""]       = "application/octet-stream";
    
    
    if (!Url.empty() && Url.find(".") == std::string::npos)
        return findExtensionByMime(m, Url);
    std::string::size_type dot = Url.find_last_of('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
    std::string exc = Url.substr(dot);
    if (m.find(exc) != m.end())
        return m[exc];
    return "application/octet-stream";
}
std::string Response::Connectionstatus(std::string con){
    if (con == "close")
        return "Connection: close\r\n\r\n";
    else
        return "Connection: keep-alive\r\n\r\n";
}
///
std::string Response::getHeaderResponse(std::string Mimetype, int size, int code){
    std::ostringstream res;
	res << "HTTP/1.1 " << code << " " << this->getCodeMessage(code) << "\r\n";
	res << "Content-Type: " << this->_MimeTypes(Mimetype) << "\r\n";
	res << "Content-Length: " << size << "\r\n";
    return res.str();
}
std::string Response::getRedirectResponse(const std::string& newUrl, int code) {
    std::ostringstream res;
    res << "HTTP/1.1 " << code << " " << this->getCodeMessage(code) << "\r\n";
    res << "Server: Webserv/1.0\r\n";
    res << "Location: " << newUrl << "\r\n";
    res << "Content-Length: 0" << "\r\n";
    res << "Content-Type: text/html\r\n";
    res << "Connection: close\r\n\r\n";
    return res.str();
}
std::string Response::getResponse(std::string path, std::map<int, std::string> error_pages){
    std::ifstream file(path.c_str());
	if (!file.is_open())
        return this->ErrorResponse(500, error_pages);

    std::ostringstream content;
	content << file.rdbuf();
	file.close();
    
    std::string headers = getHeaderResponse(path, content.str().size(), 200) + Connectionstatus("close");
    headers += content.str();

    return headers;
}
