#pragma once

#include "webserv.h"
class Request;
class Response;

class Client{
    int             _fd;
    std::string     _requBuf;
    std::string     _respoBuf;
    ssize_t          _bySent;
    time_t          _lastActive;
    char            _buffer[8192];
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
    void            setRequest(int epollFd);
    // void            setResponse();

    //
    void GetRequest(Request& req, Response& res);
};
