#include "../inc/client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
///
Client::Client():_fd(-1), _bySent(0),_lastActive(0), requCheck(false){}
Client::Client(int cliFd, sockaddr_in& cliAdd, ServerConfig& se, int cli_id):_fd(cliFd) , _cliId(cli_id), _bySent(0) ,_cliAdd(cliAdd), requCheck(false){
    this->_lastActive = time(NULL);
    this->server = se;
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
bool Client::timeOut() { return (time(NULL) - _lastActive) > 20; }

/// 
void Client::addBuffer(char *buf, ssize_t byRead){
    this->_requBuf.append(buf, byRead);
    this->_lastActive = time(NULL);
    if(this->_requBuf.find("\r\n\r\n") != std::string::npos)
        this->requCheck = true;
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

/// 
void Client::HttpRequest(){

    Request rq(_requBuf);
    Response res;
    if(!rq.isValidHeaders()){}
    if (!rq.findBestLocation(rq.getUri(), this->server)) {
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
    // else {
    //     // response.setStatusCode(405);
    //     // response.setBody("Method Not Allowed");
    // }
    // epoll_event event;
    // event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    // event.data.fd = this->_fd;
    // epoll_ctl(epollFd, EPOLL_CTL_MOD, this->_fd, &event);
}

/// METHODS : GET POST DELETE
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
void Client::GetMethod(Request& req, Response& res){
    struct stat st;

    std::cout << "dsds : " << ": " << req.getUri() << ": " << req.getPath() << std::endl;
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

}

void Client::DeleteMethod(Request& req, Response& res){
    
}