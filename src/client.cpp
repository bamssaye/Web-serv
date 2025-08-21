#include "../inc/client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
///
Client::Client():_bySent(0),_lastActive(0), _sendingFile(false), requCheck(false){}
Client::Client(ServerConfig& se, int cli_id):_cliId(cli_id), _bySent(0) , _sendingFile(false), requCheck(false){
    this->_lastActive = time(NULL);
    this->server = se;
    this->_contentLength = 0;
    this->requCheckcomp = false;
    
}
Client::~Client(){}

/// 
bool Client::dataPending() { return _bySent < static_cast<ssize_t>(_respoBuf.size()); }
const char* Client::getdataPending() { return _respoBuf.c_str() + _bySent; }
size_t Client::getSizePending() { return _respoBuf.size() - _bySent; }
bool Client::getsendingFile(){return _sendingFile;}
int Client::getID(){return _cliId;}

///
void Client::dataSent(ssize_t bySent) {
    _bySent += bySent;
    _lastActive = time(NULL);
}
bool Client::timeOut() { return (time(NULL) - _lastActive) > 50; }

void Client::addBuffer(char *buf, ssize_t byRead){
    this->_requBuf.append(buf, byRead);
    this->_lastActive = time(NULL);

    if (!this->requCheck) {
        size_t headerEnd = this->_requBuf.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            this->requCheck = true;
            size_t pos = this->_requBuf.find("Content-Length: ");
            if (pos != std::string::npos && pos < headerEnd) {
                pos += 16;
                size_t endPos = this->_requBuf.find("\r\n", pos);
                if (endPos != std::string::npos) {
                    std::string lenStr = this->_requBuf.substr(pos, endPos - pos);
                    this->_contentLength = std::atoll(lenStr.c_str());
                }
            }
        }
    }
    if (this->requCheck && !this->requCheckcomp) {
        size_t headerEnd = this->_requBuf.find("\r\n\r\n");
        
        if (headerEnd != std::string::npos) {
            size_t bodyStart = headerEnd + 4;
            size_t totalSize = this->_requBuf.size();
            size_t bodySize = (totalSize > bodyStart) ? (totalSize - bodyStart) : 0;
            if (bodySize >= this->_contentLength) {
                this->requCheckcomp = true;
                std::cerr << "REQUEST COMPLETE!" << std::endl;
            }
        }
    }
}

void Client::clearRequs(){
    this->_requBuf.clear();
    this->requCheck = false;
}
void Client::readnextChunk() {
    if (!this->_file.is_open()) return;

    if (_bySent == static_cast<ssize_t>(_respoBuf.size())) {
        _respoBuf.clear();
        _bySent = 0;
    }
    char buffer[65536];
    _file.read(buffer, 65536);
    std::streamsize bytesRead = _file.gcount();
    if (bytesRead > 0) {
        _respoBuf.append(buffer, bytesRead);
    } else {
        _file.close();
        _sendingFile = false;
    }
}
void Client::readlargeFile(std::string file, Response& res){
    this->_file.open(file.c_str(), std::ios::binary);
    if (!this->_file.is_open()) {
        this->_respoBuf = res.ErrorResponse(404);
        this->_sendingFile = false;
        return;
    }
    long long size = res.fileSize(file);
    std::string headers = res.getHeaderResponse(file, size, 200) + res.Connectionstatus("close");
    this->_respoBuf = headers;
    this->_sendingFile = true;
    this->readnextChunk();
}
/// 
void Client::HttpRequest(){

    
    Request rq(_requBuf);
    Response res;
    if(!rq.isValidHeaders()){}
    if (!rq.findBestLocation(rq.getUri(), this->server)){
        this->_respoBuf = res.ErrorResponse(403);
        return;
    }
    if (rq.loc_config.return_code != 0) {
            this->_respoBuf = res.getRedirectResponse(rq.loc_config.return_url, rq.loc_config.return_code);
        return;
    }
    rq.getFullPath(rq.getUri(), rq.loc_config);

    std::string method = rq.getMethod();
    if (method == "GET") {
        GetMethod(rq, res);
    }
    else if (method == "POST") {
        PostMethod(rq, res);
    }
    else {
        this->_respoBuf = res.ErrorResponse(501);
    }
}

/// METHODS : GET POST DELETE

void Client::GetMethod(Request& req, Response& res){
    struct stat st;

    if (stat(req.getPath().c_str(), &st) < 0){
        this->_respoBuf =  res.ErrorResponse(404); return;
    }

    if(S_ISDIR(st.st_mode)){
        std::string in = req.getPath() + "/index.html";
        if (!stat(in.c_str(), &st) && S_ISREG(st.st_mode)){
            this->_respoBuf = res.getResponse(in);
        }
        else if (req.loc_config.autoindex){
            std::string content = req.getDirContent();
            std::string headers = res.getHeaderResponse(".html",content.size(), 200) + res.Connectionstatus("close");
            this->_respoBuf = headers + content;
        }
        else {this->_respoBuf = res.ErrorResponse(403);}
        return;
    }
    if (S_ISREG(st.st_mode)){
        long long fsize = res.fileSize(req.getPath());
        if (fsize != -1 && fsize < 1000){
            this->_respoBuf =  res.getResponse(req.getPath());
            this->_sendingFile = false;
        }
        else if (fsize != -1 && fsize > 1000){
            readlargeFile(req.getPath(), res);
        }
        else{
            this->_respoBuf = res.ErrorResponse(403);
        }
    }
    else
    {
        this->_respoBuf = res.ErrorResponse(403);
    }
}

void Client::PostMethod(Request& req, Response& res){
    std::string conType = req.getHeadr("Content-Type");
    long long conlen = req.getcontentLen();
    if (req.getHeadr("Content-Length").empty() || conlen > server.client_max_body_size || conlen < 0 ){
        this->_respoBuf = res.ErrorResponse(400);
        return;
    }
    if (conType.find("multipart/form-data") != std::string::npos){
        std::vector<FormPart> content = req.MultipartBody(req.getBody(), conType);
        std::map<std::string, std::string> form;
        for (std::vector<FormPart>::const_iterator it = content.begin(); it != content.end(); ++it) {
            const FormPart& part = *it;
            if (!part.filename.empty()) {
                std::string uploadPath = res.getUploadFilename(req.loc_config.upload_path, part.filename, _toString(_lastActive));
                std::ofstream out(uploadPath.c_str(), std::ios::binary);
                if (!out.is_open()) {this->_respoBuf = res.ErrorResponse(500);return;}
                out.write(part.content.c_str(), part.content.size());
                out.close();
            } else {
                form[part.name] = part.content;
            }
        }
        std::string body = "File uploaded successfully";
        std::string headers = res.getHeaderResponse(".text",body.size(), 200) + res.Connectionstatus("close");
        this->_respoBuf = headers + body;
        return;
    }
    else if (conType.find("application/x-www-form-urlencoded") != std::string::npos){
        std::map<std::string, std::string> query = req._FormUrlDec(req.getQuery()); 
        std::map<std::string, std::string> content = req._FormUrlDec(req.getBody()); 

        std::cerr << req.getQuery() << std::endl;
        std::ostringstream que;
        std::ostringstream body;
        body << "{";
        if (!query.empty()){
            for (std::map<std::string, std::string>::iterator it = query.begin(); it != query.end(); ) {
                body << "\"" << it->first << "\": \"" << it->second << "\",";
                ++it;
            }
        }
        for (std::map<std::string, std::string>::iterator it = content.begin(); it != content.end(); ) {
            body << "\"" << it->first << "\": \"" << it->second << "\"";
            ++it;
            if (it != content.end()) {
                body << ", ";
            }
        }
        body << "}";
        std::string headers = res.getHeaderResponse(".json",body.str().size(), 200) + res.Connectionstatus("close");
        this->_respoBuf = headers + body.str();
        std::cerr << this->_respoBuf << std::endl;
    }
}
