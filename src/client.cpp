#include "../inc/client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"



Client::Client():_fd(-1), _bySent(0),_lastActive(0), requCheck(false){}

Client::Client(int cliFd, sockaddr_in& cliAdd):_fd(cliFd) ,_bySent(0) , requCheck(false){
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
template <typename T>
std::string _tString(T value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
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

    // std::ostringstream res;
	// res << "HTTP/1.1 200 OK\r\n";
	// res << "Content-Type: " << "video/mp4" << "\r\n";
	// res << "Content-Length: " << content.str().size() << "\r\n";
	// res << "\r\n";
	// res << content.str();
    // _respoBuf = 
    std::ifstream file("src/ds.jpeg", std::ios::binary);
    
    if (!file.is_open()){
        std::cout << "------=0-" << std::endl;
    }
    file.seekg(0, std::ios::end);
    std::streampos ds = file.tellg();
    file.seekg(0, std::ios::beg);
    std::ostringstream ress;
	
    while(!file.eof()){
        
        file.read(_buffer, sizeof(_buffer));
        std::streamsize bytesRead = file.gcount();
        ress << "HTTP/1.1 200 OK\r\n";
	    ress << "Content-Type: " << "image/jpeg" << "\r\n";
	    ress << "Content-Length: " << _tString(ds) << "\r\n";
	    ress << "\r\n";
        _respoBuf = ress.str();
        // 
        _respoBuf.append(_buffer, bytesRead);
        std::cout << "------------0-0-0-0-0-0-0-0-0-0-0-0-0- \n"  << _respoBuf<< std::endl;
    } 
    if (file.eof()){
        file.close();
        close(_fd);
        // sending_file = false;
    }

}

