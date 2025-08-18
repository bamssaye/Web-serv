#include "../inc/client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"



Client::Client():_fd(-1), _bySent(0),_lastActive(0), requCheck(false){}

Client::Client(int cliFd, sockaddr_in& cliAdd):_fd(cliFd) ,_bySent(0) ,_cliAdd(cliAdd), requCheck(false){
    this->_lastActive = time(NULL);
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
size_t Client::getSizePending() { return _respoBuf.size() - _bySent; }
void Client::dataSent(ssize_t bySent) {
    _bySent += bySent;
    _lastActive = time(NULL);
}
bool Client::timeOut() { return (time(NULL) - _lastActive) > 50; }

void Client::setRequest(int epollFd){

    Request rq(_requBuf);
    Response res;
    if(!rq.isValidHeaders()){}
    std::string method = rq.getMethod();//re.getMethod();
    if (method == "GET") {
        GetRequest(rq, res);
    }
    else {
        // response.setStatusCode(405);
        // response.setBody("Method Not Allowed");
    }

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

void Client::GetRequest(Request& req, Response& res){
    // struct stat st;
    // std::string Uri = req.getUri();
    // std::cerr << "READ file  : " << req.getUri().c_str()  << std::endl;
    // if (stat(req.getUri().c_str() , &st) < 0){
    //     this->_respoBuf = res.ErrorResponse(404);}
    (void)req;
    (void)res;
    // std::cout << "------------0-0-0-0-0-0-0-0-0-0-0-0-0-" << std::endl;
    std::ifstream file("mo1.mp4");
    if (file.is_open()){
        std::cout << "------------0-0-0-0-0-0-0-0-0-0-0-0-0-" << std::endl;
    }
        // std::cout << "errr" << std::endl;
    if (!file.eof()) {
        file.read(_buffer, sizeof(_buffer));
        // size_t bytesRead = file.gcount();
        _respoBuf = _buffer;
        // ssize_t sent = send(_fd, _buffer, bytesRead, 0);
    } else {
        file.close();
        close(_fd);
        // sending_file = false;
    }

}

// std::string Client::getResponse(){
//     std::ifstream file("../html/mo1.mp4");
//      if (!file.eof()) {
//         file.read(buffer, sizeof(client.buffer));
//         size_t bytesRead = client.file.gcount();
//         ssize_t sent = send(client.fd, client.buffer, bytesRead, 0);
//         if (sent == -1 && errno == EAGAIN) {
//             // السوكيت ماقدّش يكتب دابا → نستناو event آخر
//         }
//     } else {
//         // سالينا الملف
//         client.file.close();
//         close(client.fd);
//         client.sending_file = false;
//     }
// 	if (!file.is_open())
// 		return ErrorResponse(500);
//     std::ostringstream content;
// 	content << file.rdbuf();
// 	file.close();
//     std::ostringstream res;
// 	res << "HTTP/1.1 200 OK\r\n";
// 	res << "Content-Type: " << "video/mp4" << "\r\n";
// 	res << "Content-Length: " << content.str().size() << "\r\n";
// 	res << "\r\n";
// 	res << content.str();

//     std::cout   << std::string(50, '*')
//                 << res.str() << std::endl;
//     return res.str();
// }