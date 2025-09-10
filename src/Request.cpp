#include "../inc/Request.hpp"
#include "../inc/Library.hpp"

// /////
Request::Request():_boday(""),_isvalid(false), _contentLength(-1){}
Request::Request(std::string& reqMsg):_boday(""),_isvalid(false), _contentLength(-1){
    std::istringstream ss(reqMsg);
    std::string reqLine;
    std::getline(ss, reqLine);
    std::cerr << "Request Line: " << reqLine << std::endl;
    this->_parseRequestLine(reqLine);
    this->_parseHeaderFields(ss);
    size_t pos_end = reqMsg.find("\r\n\r\n");
    if (_contentLength < MAX_SIZE){
        _boday = reqMsg.substr(pos_end + 4);
    }
}
Request::~Request(){}

// ////
bool                                Request::isValidHeaders() const {   return this->_isvalid;}
std::map<std::string, std::string>  Request::getHeaders(){      return this->_headers;}
std::string                         Request::getQuery(){        return this->_Query;}
std::string                         &Request::getBody(){        return this->_boday;}
std::string                         Request::getCgipass(){      return this->_cgi_pass;}
std::string                         Request::getMethod(){       return this->_method;}
std::string                         Request::getUri(){          return this->_uriPath;}
std::string                         Request::getPath(){         return this->_path;}
int                                 Request::getcontentLen(){   return this->_contentLength;}
void                                Request::setCgipass(std::string cgi){this->_cgi_pass = cgi;}
std::string                         Request::getHeadr(std::string key){
    std::map<std::string, std::string>::iterator  it = _headers.find(key);
    return (it != _headers.end()) ? it->second : "";
}
void Request::setHeadr(std::string key, std::string value){
    this->_headers[key] = value;
}

/// /// 
void                                Request::_parseRequestLine(std::string& RqLine){
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
    if (_httpV != "HTTP/1.0" && _httpV != "HTTP/1.1")
        _isvalid = true;
}
void                                Request::_parseHeaderFields(std::istringstream& RqHeaders){
	std::string buffer;
	std::getline(RqHeaders, buffer);
	while (std::getline(RqHeaders, buffer))
	{
        // std::cerr << "Header Line: " << buffer << std::endl;
        if (buffer == "\r")
			break;
		size_t pos = buffer.find(':', 0);
		if (pos != std::string::npos){
            if (buffer.substr(0, pos) == "Content-Length"){
                size_t endPos = buffer.find("\r", pos + 2);
                if (endPos != std::string::npos) {
                    std::string lenStr = buffer.substr(pos + 2, endPos - (pos + 2));
                    this->_contentLength = Library::stoi(lenStr);
                    if (this->_contentLength > std::numeric_limits<int>::max())
                        this->_contentLength = -1;
                }
            }
            std::string value = buffer.substr(pos + 2);
			_headers[buffer.substr(0, pos)] = value.erase(value.find_last_not_of("\t"));        
        }else{
            _headers.clear();
            
            _isvalid = true;
            break;
        }
	}
    if (_method == "POST" && (getHeadr("Content-Length").empty() || getHeadr("Content-Type").empty() || _contentLength <= 0)){
        _isvalid = true;}
}

/// ///
std::string                         Request::getDirContent(){

    DIR* dir =  opendir(this->_path.c_str());
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
        if (_path.length() == 0 || _uriPath[_uriPath.length() - 1] != '/')
            content << "/";
        content << name << "\">" << name << "</a></li>\n";
    }
    closedir(dir);
    content << "</ul>";
    return content.str();
}
bool                                Request::findBestLocation(const std::string& requestPath, const ServerConfig& serverConfig) {
    size_t longestMatchLength = 0;

    for (size_t i = 0; i < serverConfig.locations.size(); ++i) {
        const LocationConfig& loc = serverConfig.locations[i];

        if (requestPath.rfind(loc.path, 0) == 0) {
            if (loc.path.length() > longestMatchLength) {
                longestMatchLength = loc.path.length();
                loc_config = loc;
            }
        }
    }
    if (longestMatchLength > 0) {
        return true;
    } else {
        loc_config = LocationConfig();
        return false;
    } 
}
void                                Request::getFullPath(const std::string &urlPath, LocationConfig &locationConfig)
{
    std::string cleanRoot = locationConfig.root;
    if (cleanRoot.size() > 1 && cleanRoot[cleanRoot.size() - 1] == '/')
        cleanRoot.erase(cleanRoot.size() - 1);

    std::string relative = urlPath;

    if (!locationConfig.path.empty() && urlPath.find(locationConfig.path) == 0) {
        relative = urlPath.substr(locationConfig.path.size());
        if (!relative.empty() && relative[0] == '/')
            relative.erase(0, 1);
    }

    this->_path = cleanRoot;
    if (!relative.empty())
        this->_path += "/" + relative;
}

/// //// FROM URL DEC PARSSING
std::map<std::string, std::string>  Request::FormUrlDec(const std::string& body) {
    std::map<std::string, std::string> full;
    std::istringstream _body(body);
    std::string l;

    while (std::getline(_body, l, '&')) {
        size_t pos = l.find('=');
        if (pos == std::string::npos){
            full.clear();
            return (full);
        }
        std::string key = Library::DecodeUrl(l.substr(0, pos));
        std::string value = Library::DecodeUrl(l.substr(pos + 1));
        full[key] = value;
    }
    return full;
}
