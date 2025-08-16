#pragma once

#include "webserv.h"

class Client{
    int             _fd;
    std::string     _requBuf;
    std::string     _respoBuf;
    ssize_t          _bySent;
    time_t          _lastActive;
    
    sockaddr_in     _cliAdd;
public:
    Client();
    Client(int cliFd, sockaddr_in& cliAdd);
    ~Client();

    //
    bool            requCheck;
    bool            dataPending();
    const char*     getdataPending();
    size_t          getSizePending();
    //
    void            addBuffer(char *buf, ssize_t byRead);
    void            clearRequs();
    void            dataSent(ssize_t bySent);
    bool            timeOut();
    //
    void            setRequest();
    void            setResponse();
};

Client::Client():_fd(-1),_bySent(0),_lastActive(0),requCheck(false){

}
Client::Client(int cliFd, sockaddr_in& cliAdd):_fd(cliFd), _cliAdd(cliAdd),_bySent(0) ,_requCheck(false){
    this->_lastActive = time(NULL);
}
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
bool Client::dataPending() { return _bySent < _respoBuf.size(); }
const char* Client::getdataPending() { return _respoBuf.c_str() + _bySent; }
size_t Client::getSizePending() { return _respoBuf.size() - _bySent; }
void Client::dataSent(ssize_t bySent) {
    _bySent += bySent;
    _lastActive = time(NULL);
}
bool Client::timeOut() { return (time(NULL) - _lastActive) > 50; }
Client::~Client(){}
void Client::setRequest(){
    
}

