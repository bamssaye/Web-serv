#include "../inc/Request.hpp"
#include "../inc/Library.hpp"

// /////
Request::Request():_boday(""),_isvalid(false), _contentLength(-1){}
Request::Request(std::string& reqMsg):_boday(""),_isvalid(false), _contentLength(-1){
    std::istringstream ss(reqMsg);
    std::string reqLine;
    std::getline(ss, reqLine);
    // std::cerr << "Request Line: " << reqLine << std::endl;
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
        if (buffer == "\r")
			break;
        std::cerr << "Request Body: " << buffer << std::endl;
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

/// //// BOUNDRY PARSSING 
std::string                         Request::ExtractBoundry(const std::string& str){
    size_t pos = str.find("boundary=");
    if (pos == std::string::npos)
        return "";
    std::string key("boundary=");
    std::string boundary = str.substr(pos + key.length());
    if (!boundary.empty() && boundary[0] == '"') {
        size_t end = boundary.find('"', 1);
        if (end != std::string::npos)
            boundary = boundary.substr(1, end - 1);
    }
    return boundary;
}
FormPart                            Request::BoundryBody(const std::string& part) {
	FormPart rzlt;

	size_t headerEnd = part.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		throw std::runtime_error("Error");

	std::string headers = part.substr(0, headerEnd);
	std::string body = part.substr(headerEnd + 4);

	std::istringstream headerStream(headers);
	std::string line;

	while (std::getline(headerStream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.find("Content-Disposition:") == 0) {
			size_t namePos = line.find("name=\"");
			if (namePos != std::string::npos) {
				size_t start = namePos + 6;
				size_t end = line.find("\"", start);
				if (end != std::string::npos)
					rzlt.name = line.substr(start, end - start);
			}

			size_t filePos = line.find("filename=\"");
			if (filePos != std::string::npos) {
				size_t start = filePos + 10;
				size_t end = line.find("\"", start);
				if (end != std::string::npos)
					rzlt.filename = line.substr(start, end - start);
			}
		} else if (line.find("Content-Type:") == 0) {
			rzlt.contentType = line.substr(13);
			while (!rzlt.contentType.empty() && rzlt.contentType[0] == ' ')
				rzlt.contentType.erase(0, 1);
		}
	}

	if (!body.empty() && body[body.size() - 1] == '\n') {
		if (body.size() >= 2 && body[body.size() - 2] == '\r')
			body = body.substr(0, body.size() - 2);
		else
			body = body.substr(0, body.size() - 1);
	}

	rzlt.content = body;
	return rzlt;
}
std::vector<FormPart>               Request::MultipartBody(const std::string& body, const std::string& conType){
    std::vector<std::string> all;
    std::string b = ExtractBoundry(conType);
    std::string del = "--" + b;
    size_t pos = 0;
    size_t next;
    std::vector<FormPart> allpart;
    std::string cont;
    
    if ( _contentLength > MAX_SIZE){
        std::ifstream   _file;
        _file.open(body.c_str(), std::ios::binary);
        if (!_file.is_open()) {allpart.clear(); return allpart;}
        cont.assign((std::istreambuf_iterator<char>(_file)),
                            (std::istreambuf_iterator<char>()));
        _file.close();
    }
    else{
        cont = body;
    }
    // std::cerr << "body: " << cont << std::endl;
    while ((next = cont.find(del, pos)) != std::string::npos) {
        if (next > pos) {
            std::string part = cont.substr(pos, next - pos);
            all.push_back(part);
        }
        pos = next + del.length();
        if (cont.substr(pos, 2) == "--")
            break;
        if (cont.substr(pos, 2) == "\r\n")
            pos += 2;
    }
    for (size_t i = 0; i < all.size(); ++i) {
        try{
        FormPart p = BoundryBody(all[i]);
        allpart.push_back(p);
        }catch(...){
            allpart.clear();
            return allpart;
        }
    }
    return allpart;
}