#pragma once

#include "webserv.h"
// #include "CgiHandler.hpp"
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
    std::ofstream   __readBuffer;
    std::string     _requfilename;

public:
    Client();
    Client(ServerConfig& se, int cli_id);
    ~Client();

    //
    bool            requCheck;
    bool            requCheckcomp;
    pid_t           cgi_pid;
    std::string     cgi_output;
    int             saveStdin;
    int             saveStdout;
    int             fdIn;
    int             fdOut;
    FILE*           fIn;
    FILE*           fOut;
    std::time_t       startTime;
    bool            cgi_running;



    ServerConfig    server;
    bool            dataPending();
    const char*     getdataPending();
    size_t          getSizePending();
    bool            getsendingFile();
    std::string     &getrequBuf() {return _requBuf;};
    int             getID();
    void            setResponse(const std::string& res);
    std::string     getrequfilename();

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

    // MEE 
    bool getRequHeaderCheck();
    void getRequBodyCheck();
    size_t getContentLength(){ return _contentLength;};
    void readlargeFileRequest(const char *buf, ssize_t byRead);
    void genefilename();

    void close_cgi();
    void get_cgi_response(int fd, std::string& output);

};
