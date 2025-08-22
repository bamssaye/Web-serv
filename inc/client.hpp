#pragma once

#include "webserv.h"
class Request;
class Response;

class Client{
    int             _cliId;
    std::string     _requBuf;
    std::string     _respoBuf;
    ssize_t          _bySent;
    time_t          _lastActive;
    std::ifstream   _file;
    bool            _sendingFile;
    size_t       _contentLength;
    //

public:
    Client();
    Client(ServerConfig& se, int cli_id);
    ~Client();

    //
    bool            requCheck;
    bool            requCheckcomp;


    ServerConfig    server;
    bool            dataPending();
    const char*     getdataPending();
    size_t          getSizePending();
    bool            getsendingFile();
    int             getID();
    //
    void            addBuffer(char *buf, ssize_t byRead);
    void            clearRequs();
    void            dataSent(ssize_t bySent);
    bool            timeOut();
    //
    void            HttpRequest();
    //
    void readnextChunk();
    void readlargeFile(std::string file, Response& res);
    //
    void GetMethod(Request& req, Response& res);
    void PostMethod(Request& req, Response& res);
    void DeleteMethod(Request& req, Response& res);
    void Cgi_call(Request& rq, Response& res);


};
