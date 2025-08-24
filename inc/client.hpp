#pragma once

#include "webserv.h"
class Request;
class Response;

class Client{
    int             _cliId;
    std::string     _requBuf;
    std::string     _respoBuf;
    ssize_t         _bySent;
    time_t          _lastActive;
    std::ifstream   _file;
    bool            _sendingFile;
    int             _contentLength;
    std::ofstream   _readBuffer;
    std::string     _requfilename;

public:
    Client(ServerConfig& se, int cli_id);
    ~Client();
    // ///
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
    /// /////

    ServerConfig    server;
    bool            dataPending();
    const char*     getdataPending();
    size_t          getSizePending();
    bool            getsendingFile();
    std::string     &getrequBuf();
    int             getID();
    void            setResponse(const std::string& res);
    std::string     getrequfilename();
    //
    void            addBuffer(char *buf, ssize_t byRead);
    void            dataSent(ssize_t bySent);
    bool            timeOut();
    //
    void            HttpRequest();
    //
    void            readnextChunk();
    void            readlargeFile(std::string file, Response& res);
    //
    void            GetMethod(Request& req, Response& res);
    void            PostMethod(Request& req, Response& res);
    void            DeleteMethod(Request& req);
    void            Cgi_call(Request& rq);
    //
    bool            getRequHeaderCheck();
    void            getRequBodyCheck();
    int             getContentLength();
    void            readlargeFileRequest(const char *buf, ssize_t byRead);
    void            genefilename();
    void            close_cgi();
    void            get_cgi_response(int fd, std::string& output);
};
