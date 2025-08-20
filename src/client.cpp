#include "../inc/client.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include "../inc/CgiHandler.hpp"



Client::Client():_fd(-1), _bySent(0),_lastActive(0), requCheck(false){}

Client::Client(int cliFd, sockaddr_in& cliAdd, ServerConfig& server, int clientId):_fd(cliFd) ,_ClieId(clientId) ,_bySent(0) , requCheck(false), _server(server){
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
    if (!rq.findBestLocation(rq.getUri(), this->_server)) {
        this->_respoBuf = res.ErrorResponse(403);
        epoll_event event;
        event.events = EPOLLOUT | EPOLLET;
        event.data.fd = this->_fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, this->_fd, &event);
        return;
        // return; // checki wax atzid xi haja qebk mat returni hit benti lia katbdel f epoll
    }
    if (rq.loc_config.return_code != 0) {
        this->_respoBuf = rq.createRedirectResponse(rq.loc_config.return_url, rq.loc_config.return_code);
        epoll_event event;
        event.events = EPOLLOUT | EPOLLET;
        event.data.fd = this->_fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, this->_fd, &event);
        return;
    }
    rq.getFullPath(rq.getUri(), rq.loc_config);
    if (check_cgi_executable(rq)) {
            CgiHandler cgi(rq);
            std::string output = cgi.executeCgi(rq);
            if (output.empty() || output == "Status: 500\r\n\r\n")
                this->_respoBuf = res.ErrorResponse(500);
            this->_respoBuf = parseCgiOutput(output, rq);
            // return;
        }
    
    std::string method = rq.getMethod();//re.getMethod();
    if (method == "GET") {
        // GetRequest(rq, res);
    }
    else if (method == "POST") {
        // Handle POST request
        // this->setResponse();
    }
    else if (method == "DELETE") {
        DeleteMethod(rq._path, res); 
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
std::string _tString(T value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
void Client::GetRequest(Request& req, Response& res){
    // struct stat st;
    this->_respoBuf = res.ErrorResponse(200);
    // std::string Uri = req.getUri();
    // std::cerr << "READ file  : " << req.getUri().c_str()  << std::endl;
    // if (stat(req.getUri().c_str() , &st) < 0){
    //     this->_respoBuf = res.ErrorResponse(404);}
    (void)req;
    (void)res;
    (void)_buffer;
    // std::cout << "------------0-0-0-0-0-0-0-0-0-0-0-0-0-" << std::endl;

    // std::ostringstream res;
	// res << "HTTP/1.1 200 OK\r\n";
	// res << "Content-Type: " << "video/mp4" << "\r\n";
	// res << "Content-Length: " << content.str().size() << "\r\n";
	// res << "\r\n";
	// res << content.str();
    // _respoBuf = 
    // std::ifstream file("src/ds.jpeg", std::ios::binary);
    
    // if (!file.is_open()){
    //     std::cout << "------=0-" << std::endl;
    // }
    // file.seekg(0, std::ios::end);
    // std::streampos ds = file.tellg();
    // file.seekg(0, std::ios::beg);
    // std::ostringstream ress;
	
    // // while(!file.eof()){
        
    //     file.read(_buffer, sizeof(_buffer));
    //     std::streamsize bytesRead = file.gcount();
    //     ress << "HTTP/1.1 200 OK\r\n";
	//     ress << "Content-Type: " << "image/jpeg" << "\r\n";
	//     ress << "Content-Length: " << _tString(ds) << "\r\n";
	//     ress << "\r\n";
    //     _respoBuf = ress.str();
    //     // 
    //     _respoBuf.append(_buffer, bytesRead);
    //     std::cout << "------------0-0-0-0-0-0-0-0-0-0-0-0-0- \n"  << _respoBuf<< std::endl;
    // } 
    // if (file.eof()){
    //     file.close();
    //     close(_fd);
    //     // sending_file = false;
    // }

}


void Client::DeleteMethod(const std::string& fpath, Response& res) {
    struct stat st;
    if (stat(fpath.c_str(), &st) < 0) {
        this->_respoBuf = res.ErrorResponse(404);
    }
    if (access(fpath.c_str(), W_OK) != 0) {
        this->_respoBuf = res.ErrorResponse(403);
    }
    if (!S_ISREG(st.st_mode)) {
        this->_respoBuf = res.ErrorResponse(405);
    }

    if (remove(fpath.c_str()) != 0) {
        this->_respoBuf = res.ErrorResponse(500);
    }
    std::cout << res.getCodeMessage(204) << std::endl;
    this->_respoBuf = res.ErrorResponse(204);
}