#include "../inc/client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include "../inc/CgiHandler.hpp"
#include "../inc/Library.hpp"
///  ////
Client::Client(ServerConfig& se, int cli_id):_cliId(cli_id), _bySent(0) , _sendingFile(false), requCheck(false){
    this->_lastActive = time(NULL);
    this->server = se;
    this->cgi_pid = -1;
    this->cgi_output = "";
    this->saveStdin = -1;
    this->saveStdout = -1;
    this->fdIn = -1;
    this->fdOut = -1;
    this->startTime = 0;
    this->cgi_running = false;
    this->_contentLength = 0;
    this->requCheckcomp = false;
    this->_requfilename = Library::geneFileName(_cliId, _lastActive);
    this->_readBuffer.open(_requfilename.c_str(), std::ios::binary);
}
Client::~Client(){
    if(this->_readBuffer.is_open()){
        this->_readBuffer.close();
    }
    remove(_requfilename.c_str());
}

/// //////////
bool        Client::dataPending() {     return _bySent < static_cast<ssize_t>(_respoBuf.size()); }
const char* Client::getdataPending() {  return _respoBuf.c_str() + _bySent; }
size_t      Client::getSizePending() {  return _respoBuf.size() - _bySent; }
bool        Client::getsendingFile(){   return _sendingFile;}
int         Client::getID(){            return _cliId;}
void        Client::dataSent(ssize_t bySent) { _bySent += bySent; _lastActive = time(NULL);}
bool        Client::timeOut() {         return (time(NULL) - _lastActive) > 10; }
std::string Client::getrequfilename(){  return this->_requfilename;}
int         Client::getContentLength(){ return _contentLength;}
std::string &Client::getrequBuf(){      return _requBuf;};
void        Client::setResponse(const std::string& res) {
    this->_respoBuf = res;
    this->_bySent = 0;
}

/// ////////// READ DATA 
bool Client::getRequHeaderCheck(){
    this->_lastActive = time(NULL);
    if (this->requCheck){return true;}
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
                    this->_contentLength = Library::stoi(lenStr);
                    if (this->_contentLength > std::numeric_limits<int>::max())
                        this->_contentLength = -1;
                }
            }
            return true;
        }
    }
    return false;
}
void Client::readlargeFileRequest(const char *buf, ssize_t byRead){
    this->_lastActive = time(NULL);
    if (!this->_readBuffer.is_open()) {
        this->_respoBuf = Library::ErrorResponse(500, this->server.error_pages);
        this->_sendingFile = false;
        return;
    }
    _readBuffer.write(buf, byRead);
    if (this->requCheck && !this->requCheckcomp) {
        size_t headerEnd = this->_requBuf.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            int bodySize = Library::FileSize(this->_requfilename);
            if (bodySize  >= this->_contentLength) {
                this->requCheckcomp = true;
            }
        }
    }
}
void Client::addBuffer(char *buf, ssize_t byRead){
    this->_requBuf.append(buf, byRead);
    getRequHeaderCheck();
    if (this->requCheck && !this->requCheckcomp) {
        size_t headerEnd = this->_requBuf.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            size_t bodyStart = headerEnd + 4;
            size_t totalSize = this->_requBuf.size();
            int bodySize = (totalSize > bodyStart) ? (totalSize - bodyStart) : 0;
            if (bodySize >= this->_contentLength) {
                this->requCheckcomp = true;
            }
        }
    }
}
void Client::readnextChunk() {
    if (!this->_file.is_open()){
        this->_respoBuf = Library::ErrorResponse(404, this->server.error_pages);
        this->_sendingFile = false;
        return;
    }
    _lastActive = time(NULL);
    if (_bySent == static_cast<ssize_t>(_respoBuf.size())) {
        _respoBuf.clear();
        _bySent = 0;
    }
    char buffer[8192];
    _file.read(buffer, 8192);
    std::streamsize bytesRead = _file.gcount();
    if (bytesRead > 0) {
    //     long long size = bytesRead + _respoBuf.size();
    //     Response res;
    // std::string headers = res.getHeaderResponse(this->filename, size, 200) + res.Connectionstatus("clo se");
    // this->_respoBuf = headers;
    // if (Library::FileSize(this->filename) > size) {
    //     this->_sendingFile = true;
    // }
    // this->_sendingFile = true;

        _respoBuf.append(buffer, bytesRead);
    } else {
        _file.close();
        _sendingFile = false;
    }
}
void Client::readlargeFile(Request& req, Response& res){
    this->_file.open(req.getPath().c_str(), std::ios::binary);
    if (!this->_file.is_open()) {
        this->_respoBuf = Library::ErrorResponse(404, this->server.error_pages);
        this->_sendingFile = false;
        return;
    }
    // (void)res;
    if (req.getHeadr("Range") != "") 
    {
        // Handle Range requests for partial content delivery
        std::string rangeHeader = req.getHeadr("Range");
        size_t eqPos = rangeHeader.find('=');
        if (eqPos != std::string::npos) {
            std::string byteRange = rangeHeader.substr(eqPos + 1);
            size_t dashPos = byteRange.find('-');
            if (dashPos != std::string::npos) {
                std::string startStr = byteRange.substr(0, dashPos);
                std::string endStr = byteRange.substr(dashPos + 1);
                long long start = startStr.empty() ? 0 : Library::stoi(startStr);
                long long end = endStr.empty() ? Library::FileSize(req.getPath()) - 1 : Library::stoi(endStr);
                if (start >= 0 && end >= start) {
                    long long fileSize = Library::FileSize(req.getPath());
                    if (end >= fileSize) end = fileSize - 1;
                    long long contentLength = end - start + 1;
                    this->_file.seekg(start);
                    std::ostringstream headers;
                    headers << "HTTP/1.1 206 Partial Content\r\n";
                    headers << "Content-Type: " << res._MimeTypes(req.getPath()) << "\r\n";
                    headers << "Content-Length: " << contentLength << "\r\n";
                    headers << "Content-Range: bytes " << start << "-" << end << "/" << fileSize << "\r\n";
                    headers << res.Connectionstatus("close");
                    this->_respoBuf = headers.str();
                    this->_sendingFile = true;
                    this->readnextChunk();
                    return;
                }
            }
        }
    }
    this->filename = req.getPath();
    long long size = Library::FileSize(req.getPath());
    std::string headers = res.getHeaderResponse(req.getPath(), size, 200) + res.Connectionstatus("close");
    this->_respoBuf = headers;
    this->_sendingFile = true;
    this->readnextChunk();
}

/// HTTP REQUEST
void Client::HttpRequest(){

    Request rq(_requBuf);
    Response res;
    if(rq.isValidHeaders()){
        this->_respoBuf =  res.ErrorResponse(403, this->server.error_pages); return;
    }
    if (!rq.findBestLocation(rq.getUri(), this->server)){
        this->_respoBuf = res.ErrorResponse(404, this->server.error_pages);
        return;
    }
    if (!std::count(rq.loc_config.allowed_methods.begin(), rq.loc_config.allowed_methods.end(), rq.getMethod())
        && rq.loc_config.allowed_methods.size() > 0) {
        this->_respoBuf = res.ErrorResponse(400, this->server.error_pages);
        return;
    }
    if (rq.loc_config.return_code != 0) {
            this->_respoBuf = res.getRedirectResponse(rq.loc_config.return_url, rq.loc_config.return_code);
        return;
    }
    rq.getFullPath(rq.getUri(), rq.loc_config);
    if (check_cgi_executable(rq)) {
        Cgi_call(rq);
        return;
    }
    std::string method = rq.getMethod();
    
    if (method == "GET") {
        GetMethod(rq, res);
    }
    else if (method == "POST") {
        PostMethod(rq, res);
    }
    else if (method == "DELETE") {
        DeleteMethod(rq);
    }
    else {
        this->_respoBuf = res.ErrorResponse(501, this->server.error_pages);
    }
}

/// METHODS : GET POST DELETE
void Client::GetMethod(Request& req, Response& res){
    struct stat st;

    if (stat(req.getPath().c_str(), &st) < 0){
        this->_respoBuf =  res.ErrorResponse(404, this->server.error_pages); return;
    }
    if(S_ISDIR(st.st_mode)){
        std::string in = req.getPath() + "/" + req.loc_config.index;
        if (!stat(in.c_str(), &st) && S_ISREG(st.st_mode)){
            this->_respoBuf = res.getResponse(in);
        }
        else if (req.loc_config.autoindex){
            std::string content = req.getDirContent();
            std::string headers = res.getHeaderResponse(".html",content.size(), 200) + res.Connectionstatus("close");
            this->_respoBuf = headers + content;
        }
        else {this->_respoBuf = res.ErrorResponse(403, this->server.error_pages);}
        return;
    }
    if (S_ISREG(st.st_mode)){
        long long fsize = Library::FileSize(req.getPath());
        if (fsize != -1 && fsize < MAX_SIZE){
            this->_respoBuf =  res.getResponse(req.getPath());
            this->_sendingFile = false;
        }
        else if (fsize != -1 && fsize > MAX_SIZE){
            readlargeFile(req, res);
        }
        else{
            this->_respoBuf = res.ErrorResponse(403, this->server.error_pages);
        }
    }
    else
    {
        this->_respoBuf = res.ErrorResponse(403, this->server.error_pages);
    }
}
void Client::PostMethod(Request& req, Response& res){
    struct stat st;
    if (stat(req.loc_config.upload_path.c_str(), &st) < 0){
        this->_respoBuf =  res.ErrorResponse(404, this->server.error_pages); return;
    }
    std::string conType = req.getHeadr("Content-Type");
    long long conlen = req.getcontentLen();
    if (conlen > server.client_max_body_size){
        this->_respoBuf = res.ErrorResponse(400, this->server.error_pages);
        return;
    }
    //res._MimeTypes(req.getHeadr("Content-Type"))
    std::string cotype = res._MimeTypes(req.getHeadr("Content-Type"));
    if (conType.find("application/x-www-form-urlencoded") != std::string::npos){
        std::map<std::string, std::string> query = req.FormUrlDec(req.getQuery()); 
        std::map<std::string, std::string> content = req.FormUrlDec(req.getBody()); 
        std::string body = Library::getJsonResponse(query, content);
        std::string headers = res.getHeaderResponse(".json", body.size(), 200) + res.Connectionstatus("close");
        this->_respoBuf = headers + body;
    }
    else if (cotype != "-1"){
        std::string conTy = res._MimeTypes(req.getHeadr("Content-Type"));
        std::string uploadPath =  Library::getUploadFilename(req.loc_config.upload_path, conTy, _toString(_lastActive));
        std::cerr << "Upload Path: " << uploadPath << std::endl;
        int fd = open(uploadPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {this->_respoBuf = res.ErrorResponse(500, this->server.error_pages);return;}
        if (_contentLength > MAX_SIZE){
            int fdtmp = open(_requfilename.c_str(), O_RDONLY);
            if (fdtmp == -1) {this->_respoBuf = res.ErrorResponse(500, this->server.error_pages);return;}
            dup2(fd, fdtmp);
            close(fd);
            close(fdtmp);
        }
        else{
            write(fd, req.getBody().c_str(), req.getBody().size());
            close(fd);
        }
        this->_respoBuf = res.ErrorResponse(201, this->server.error_pages);
    }
    else {
        this->_respoBuf = res.ErrorResponse(403, this->server.error_pages);
    }
}
void Client::DeleteMethod(Request& req) {
    struct stat st;
    if (stat(req.getPath().c_str(), &st) < 0) {
        this->_respoBuf = Library::ErrorResponse(404, this->server.error_pages);
        return;
    }
    if (access(req.getPath().c_str(), W_OK) != 0) {
        this->_respoBuf = Library::ErrorResponse(403, this->server.error_pages);
        return;
    }
    if (!S_ISREG(st.st_mode)) {
        this->_respoBuf = Library::ErrorResponse(403, this->server.error_pages);
        return;
    }

    if (remove(req.getPath().c_str()) != 0) {
        this->_respoBuf = Library::ErrorResponse(500, this->server.error_pages);
        return;
    }
    this->_respoBuf = Library::ErrorResponse(204, this->server.error_pages);
}

/// /////// CGI FUN
void Client::Cgi_call(Request& rq){
    CgiHandler cgi(rq);
    if (!cgi.executeCgi(rq, *this)) {
        this->_respoBuf = Library::ErrorResponse(500, this->server.error_pages);
        this->cgi_running = false;
        return;
    }
}
void Client::get_cgi_response(int fd, std::string& output) {
    lseek(fd, 0, SEEK_SET);
    char buffer[CGI_BUFSIZE];
    int bytes;
    while ((bytes = read(fd, buffer, CGI_BUFSIZE - 1)) > 0) {
        buffer[bytes] = '\0';
        output += buffer;
    }
    return ;
}
void Client::close_cgi() {
    dup2(saveStdin, STDIN_FILENO);
    dup2(saveStdout, STDOUT_FILENO);
    fclose(fIn);
    fclose(fOut);
    close(saveStdin);
    close(saveStdout);
}
