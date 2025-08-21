#include "../inc/Response.hpp"

/// 
Response::Response(){
    this->_cMessage = _CodeMessage();
    // this->_MimeTypes = _initMimeTypes();
}
Response::~Response(){}

/// 
std::string Response::getCodeMessage(int code){
    std::map<int, std::string>::const_iterator it = _cMessage.find(code);
    return (it != _cMessage.end()) ? it->second : "Unknown Error";
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
std::string Response::_MimeTypes(const std::string &Url){
    std::map<std::string, std::string> m;
    m.insert(std::make_pair(".html", "text/html"));
    m.insert(std::make_pair(".htm",  "text/html"));
    m.insert(std::make_pair(".css",  "text/css"));
    m.insert(std::make_pair(".js",   "application/javascript"));
    m.insert(std::make_pair(".json", "application/json"));
    m.insert(std::make_pair(".xml",  "application/xml"));
    m.insert(std::make_pair(".txt",  "text/plain"));
    m.insert(std::make_pair(".csv",  "text/csv"));
    m.insert(std::make_pair(".png",  "image/png"));
    m.insert(std::make_pair(".jpg",  "image/jpeg"));
    m.insert(std::make_pair(".jpeg", "image/jpeg"));
    m.insert(std::make_pair(".gif",  "image/gif"));
    m.insert(std::make_pair(".bmp",  "image/bmp"));
    m.insert(std::make_pair(".webp", "image/webp"));
    m.insert(std::make_pair(".svg",  "image/svg+xml"));
    m.insert(std::make_pair(".ico",  "image/x-icon"));
    m.insert(std::make_pair(".woff",  "font/woff"));
    m.insert(std::make_pair(".woff2", "font/woff2"));
    m.insert(std::make_pair(".ttf",   "font/ttf"));
    m.insert(std::make_pair(".otf",   "font/otf"));
    m.insert(std::make_pair(".eot",   "application/vnd.ms-fontobject"));
    m.insert(std::make_pair(".mp3", "audio/mpeg"));
    m.insert(std::make_pair(".wav", "audio/wav"));
    m.insert(std::make_pair(".ogg", "audio/ogg"));
    m.insert(std::make_pair(".mp4",  "video/mp4"));
    m.insert(std::make_pair(".webm", "video/webm"));
    m.insert(std::make_pair(".ogv",  "video/ogg"));
    m.insert(std::make_pair(".avi",  "video/x-msvideo"));
    m.insert(std::make_pair(".mov",  "video/quicktime"));
    m.insert(std::make_pair(".mkv",  "video/x-matroska"));
    m.insert(std::make_pair(".zip", "application/zip"));
    m.insert(std::make_pair(".tar", "application/x-tar"));
    m.insert(std::make_pair(".gz",  "application/gzip"));
    m.insert(std::make_pair(".bz2", "application/x-bzip2"));
    m.insert(std::make_pair(".7z",  "application/x-7z-compressed"));
    m.insert(std::make_pair(".rar", "application/vnd.rar"));
    m.insert(std::make_pair("", "application/octet-stream"));

    std::string::size_type dot = Url.find_last_of('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
    std::string exc = Url.substr(dot);
    if (m.find(exc) != m.end())
        return m[exc];
    return "application/octet-stream";
}

long long Response::fileSize(std::string filePath){
    std::ifstream _file;
    _file.open(filePath.c_str(), std::ios::binary);
    if (!_file.is_open()) {
        return -1;
    }
    _file.seekg(0, std::ios::end);
    std::streampos size = _file.tellg();
    _file.seekg(0, std::ios::beg);
    _file.close();
    return size;
}
std::string Response::Connectionstatus(std::string con){
    if (con == "close")
        return "Connection: close\r\n\r\n";
    else
        return "Connection: keep-alive\r\n\r\n";
}

///

std::string Response::ErrorResponse(int code){
        std::ostringstream res;
        // std::string body = this->getHtmlContent(code);
        std::string body = this->getCodeMessage(code);
        res << "HTTP/1.1 " << code << " " << this->getCodeMessage(code) << "\r\n";
        res << "Content-Type: text/html\r\n";
        res << "Content-Length: " << body.size() << "\r\n\r\n";
        res << body;
        return res.str();
}
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

std::string Response::getResponse(std::string path){
    std::ifstream file(path.c_str());
	if (!file.is_open())
        return this->ErrorResponse(500);
    
    std::ostringstream content;
	content << file.rdbuf();
	file.close();
    
    std::string headers = getHeaderResponse(path, content.str().size(), 200) + Connectionstatus("close");
    headers += content.str();

    return headers;
}

std::string Response::_extratexcFilename(std::string filename){
    std::string::size_type dot = filename.find_last_of('.');
    if (dot == std::string::npos)
        return "";
    std::string exc = filename.substr(dot);
    return exc;
}
std::string Response::_extratFilename(std::string filename){
    std::string::size_type dot = filename.find_last_of('.');
    if (dot == std::string::npos)
        return filename;
    std::string exc = filename.substr(0, dot);
    return exc;
}
std::string Response::getUploadFilename(std::string& UriPath, std::string filename, std::string rd){
    std::string file;
    
    file = UriPath + "/";
    file += _extratFilename(filename);
    file += _toString(rd);
    file += _extratexcFilename(filename);
    return file;
}