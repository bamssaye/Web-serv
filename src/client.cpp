#include "../inc/client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"



Client::Client():_fd(-1), _bySent(0),_lastActive(0), _sendingFile(false), requCheck(false){}

Client::Client(int cliFd, sockaddr_in& cliAdd):_fd(cliFd) ,_bySent(0) ,  _sendingFile(false), requCheck(false){
    this->_lastActive = time(NULL);
    this->_cliAdd = cliAdd;
}

Client::~Client(){}

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
bool Client::dataPending() { return _bySent < static_cast<ssize_t>(_respoBuf.size()); }
const char* Client::getdataPending() { return _respoBuf.c_str() + _bySent; }
bool Client::getsendingFile(){return _sendingFile;}
size_t Client::getSizePending() { return _respoBuf.size() - _bySent; }
void Client::dataSent(ssize_t bySent) {
    _bySent += bySent;
    _lastActive = time(NULL);
}
bool Client::timeOut() { return (time(NULL) - _lastActive) > 50; }

void Client::loadNextChunk(){
        if (!this->_file.is_open()) return;

        char buffer[8192];
        _file.read(buffer, sizeof(buffer));
        std::streamsize bytesRead = _file.gcount();

        if (bytesRead > 0) {
            _respoBuf.append(buffer, bytesRead);
        } else {
            _file.close(); 
            _sendingFile = false;
        }
}

void Client::setRequest(int epollFd){

    Request rq(_requBuf);
    Response res;
    if(!rq.isValidHeaders()){}
    std::string method = rq.getMethod();//re.getMethod();
    if (method == "GET") {
        GetRequest(rq, res);
    }
    // else {
    //     // response.setStatusCode(405);
    //     // response.setBody("Method Not Allowed");
    // }

    // if (rq.isValidHeaders()){
    //     // if (is cgi ){}
        
    // }
    // else {
    //     // generate error 
    // }

    epoll_event event;
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.fd = this->_fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, this->_fd, &event);
}
template <typename T> 
std::string _tString(T value){
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void Client::GetRequest(Request& req, Response& res){
    // struct stat st;
    (void)req;
    (void)res;
    this->_file.open("src/mor4.mp4", std::ios::binary);
    if (!this->_file.is_open()) {
        this->_respoBuf = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        this->_sendingFile = false;
        return;
    }
    this->_file.seekg(0, std::ios::end);
    std::streampos ds = this->_file.tellg();
    this->_file.seekg(0, std::ios::beg);
    std::ostringstream ress;
    ress << "HTTP/1.1 200 OK\r\n";
    ress << "Content-Type: " << "video/mp4" << "\r\n";
    ress << "Content-Length: " << ds << "\r\n";
    // ress << "Connection: close\r\n";
    ress << "\r\n";
    this->_respoBuf = ress.str();
    this->_sendingFile = true;
    this->loadNextChunk();
}

