#include "../inc/Request.hpp"

Request::Request(std::string& reqMsg):_boday(""),_isvalid(false), _contentLength(-1){
    std::cout << reqMsg << std::endl;
    std::istringstream ss(reqMsg);
    std::string reqLine;
    std::getline(ss, reqLine);
    this->_parseRequestLine(reqLine);
    this->_parseHeaderFields(ss);
    size_t pos_end = reqMsg.find("\r\n\r\n");
	_boday = reqMsg.substr(pos_end + 4);
}
Request::~Request(){}

void Request::_parseRequestLine(std::string& RqLine){
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
        // if (i == 3)
        //     _isvalid = true;
        if (Methods[i] == _method)
            break;
    }
    if (_httpV != "HTTP/1.0" && _httpV != "HTTP/1.1")
        _isvalid = true;
}

void Request::_parseHeaderFields(std::istringstream& RqHeaders){
	std::string buffer;
	std::getline(RqHeaders, buffer);
	while (std::getline(RqHeaders, buffer))
	{
		if (buffer == "\r")
			break;
		size_t pos = buffer.find(':', 0);
		if (pos != std::string::npos){
            if ("Content-Lenght" == buffer.substr(0, pos - 1))
                _contentLength = std::atoll(buffer.substr(0, pos + 2).c_str());
            std::string value = buffer.substr(pos + 2);
			_headers[buffer.substr(0, pos)] = value.erase(value.find_last_not_of("\t"));
        
        }else{
            _headers.clear();
            _isvalid = true;
            break;
        }
	}
}

bool Request::isValidHeaders() const { return this->_isvalid;}

std::string Request::getHeadr(std::string& key){
    std::map<std::string, std::string>::iterator  it = _headers.find(key);
    return (it != _headers.end()) ? it->second : "";
}

std::string Request::getListContent() const{

    DIR* dir =  opendir(this->_uriPath.c_str());
    if (!dir)
        return ("");
    struct dirent* entry;
    std::ostringstream content;
    content << "<h1>Index of " << _uriPath << "</h1>\n<ul>\n";
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == ".") continue;
        if (entry->d_type == DT_DIR)
            name += "/";
        content << "<li><a href=\"";
        content << _uriPath;
        if (_uriPath.length() == 0 || _uriPath[_uriPath.length() - 1] != '/')
            content << "/";
        content << name << "\">" << name << "</a></li>\n";
    }
    closedir(dir);
    content << "</ul>";
    return content.str();
}