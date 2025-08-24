#include "../inc/server.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include "../inc/CgiHandler.hpp"

Server::Server (ServerConfig& servers) : InfoSocket(), _cliCount(0), _oP(1), _server(servers){
    this->initServer();
}
Server::~Server(){
    for (std::map<int, Client*>::iterator it = _Clients.begin(); it != _Clients.end(); ++it)
        delete it->second;
    _Clients.clear();
}

InfoSocket::InfoSocket():_fd(-1), port(0), ip(0){
    memset(&this->addr, 0, sizeof(this->addr));
    memset(&this->sock_event, 0, sizeof(this->sock_event));
}
InfoSocket::~InfoSocket(){}

void Server::_Msg(std::string m){ std::cout << m << std::endl;}
void Server::_MsgErr(std::string m){ std::cerr << m << std::endl;}

void Server::initServer(){
    if((this->_epollFd = epoll_create1(EPOLL_CLOEXEC)) == -1){
        throw std::runtime_error("epoll_create1 failed");}
    std::vector<std::pair<unsigned long, unsigned short> >::iterator it = _server.listen.begin();
    while (it != _server.listen.end()){
        unsigned long host = it->first ; int port = it->second; int i = 0;
        int serveSize = _server.listen.size();
        InfoSocket sockets[serveSize];
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw std::runtime_error("socket failed");
        this->_listfd.push_back(fd);
        sockets[i].setSocket(fd, port, host, AF_INET);
        if (this->_setNonBlocking(fd) == -1)
            throw std::runtime_error("setNonBlocking failed");
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &this->_oP, sizeof(this->_oP)) < 0)
            throw std::runtime_error("setsockopt failed");
        if (bind(fd, (sockaddr *)&sockets[i].getSockaddr(), sockets[i].getsocklen()) < 0)
            throw std::runtime_error("bind failed");
        if (listen(fd, SOMAXCONN) < 0)
            throw std::runtime_error("listen failed");
        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &sockets[i++].getSockEvent()) == -1)
            throw std::runtime_error("epoll_ctl failed");
        ++it;
    }
    std::cout << "Server initialized." << std::endl;
}

void Server::runServer(){
    this->_Msg("Server listening.");
    while(true){
       int nFds = epoll_wait(_epollFd, _events, MAX_EVENTS, -1);
       if (nFds == -1){
            if (errno == EINTR)
                continue;
            throw std::runtime_error("epoll_wait faild.");
       }
       for (int i = 0; i < nFds ; i++){
        _handleEvent(_events[i]);
       }
       checkTimeouts();
    }
}
void Server::_handleEvent(const epoll_event& ev){
    int ev_fd = ev.data.fd;

    if (ev.events & (EPOLLERR | EPOLLHUP)){
        _MsgErr("Client Error : " + _toString(ev_fd));
        _closeCon(ev_fd);
        return;
    }
    if (_isNewClient(ev_fd)){
        if (ev.events & EPOLLIN){ 
            _AcceptCon(ev_fd); 
        }
    }
    if (ev.events & EPOLLIN){ 
        _ClientRead(ev_fd);
    }
    else if (ev.events & EPOLLOUT){
        _ClientWrite(ev_fd);
    }
}


void Server::_writeEvent(int epollFd, int fd){
    epoll_event event;
    event.events = EPOLLOUT ;
    event.data.fd = fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}
void Server::_readEvent(int epollFd, int fd){
    epoll_event event;
    event.events = EPOLLIN ;
    event.data.fd = fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}

void Server::_ReadContent(char *buf, ssize_t byRead, int cliFd){
    
    if (_Clients[cliFd]->getRequHeaderCheck()){
        size_t len = _Clients[cliFd]->getContentLength();
        if (len > MAX_SIZE){
            _Clients[cliFd]->readlargeFileRequest(buf, byRead);
            // _Clients[cliFd]->addBuffer(buf, byRead);// 3lach kat3mr f string
        }
        else{
            _Clients[cliFd]->addBuffer(buf, byRead);
        }
    }
    else{
        _Clients[cliFd]->addBuffer(buf, byRead);
        size_t headerEnd = _Clients[cliFd]->getrequBuf().find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            size_t bodyStart = headerEnd + 4;
            std::string body = _Clients[cliFd]->getrequBuf().substr(bodyStart);
            _Clients[cliFd]->readlargeFileRequest(body.c_str(), body.size());
        } 
        // std::cerr << "req.getQuery(1)" << std::endl;      
    }
}
void Server::_ClientRead(int cliFd){
    char buffer[BUFFER_SIZE];
    ssize_t bytes;

   
    if (_Clients.find(cliFd) == _Clients.end())
        return;
    bytes = recv(cliFd, buffer, BUFFER_SIZE, 0);
    //  _Clients[cliFd]->addBuffer(buffer, bytes);
    _ReadContent(buffer, bytes, cliFd);
    if (_Clients[cliFd]->requCheck && _Clients[cliFd]->requCheckcomp){
            _Clients[cliFd]->HttpRequest();
            _writeEvent(this->_epollFd, cliFd);
    } 
    if (bytes == 0){
        this->_closeCon(cliFd);   
    }
}
void Server::_ClientWrite(int cliFd){
    std::map<int, Client *>::iterator it = _Clients.find(cliFd);
    if (it == _Clients.end())
        return;
    Client* client = it->second;
    while (client->dataPending()) {
        ssize_t bySent = send(cliFd, client->getdataPending(), client->getSizePending(), MSG_NOSIGNAL);
        if (bySent <= 0) {
            break;
        }
        client->dataSent(bySent);
        if (!client->dataPending() && client->getsendingFile()) {
            client->readnextChunk();
        }
    }
    if (!client->getdataPending() && !client->getsendingFile()){
        _readEvent(_epollFd, cliFd);
    }
}

void Server::_AcceptCon(int FdServer){
	sockaddr_in cliAdd;
	socklen_t cliAddLen = sizeof(cliAdd);

	while (true){
		int cliFd = accept(FdServer, (sockaddr*)&cliAdd, &cliAddLen);
		if (cliFd == -1){
			if (errno == EAGAIN || errno == EWOULDBLOCK){break;}
			std::string msg = "Accept failed: ";
			this->_MsgErr(_toString(msg + strerror(errno)));
			break;
		}
		if (_setNonBlocking(cliFd) == -1){
			this->_MsgErr("Client socket Failed to set non-blocking.");
			close(cliFd);
			continue;
		}
		epoll_event cliEvent;
		cliEvent.data.fd = cliFd;
		cliEvent.events = EPOLLIN;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, cliFd, &cliEvent) == -1){
			_MsgErr("Error: epoll_ctl ADD new client failed");
			close(cliFd);
			continue;
		}
        _cliCount++;
		_Clients[cliFd] = new Client(_server ,_cliCount);
        this->_Msg("Client ID=[" + _toString(_cliCount) + "] Connected");
	}
}

void Server::_closeCon(int FdClient){
    int id = 0;
    std::map<int, Client*>::iterator it = _Clients.find(FdClient);
    if (it != _Clients.end()) {
        id = it->second->getID();
        int fd = it->first;
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        delete it->second;
       _Clients.erase(it);
       this->_Msg("Connection closed ID=[" + _toString(id) + "]");
    }
    
}
void Server::checkTimeouts() {
    std::map<int, Client*>::iterator it = _Clients.begin();
    while (it != _Clients.end()) {
        bool should_close = false;
        
       if (it->second->cgi_running){

            if (waitpid(it->second->cgi_pid, NULL, WNOHANG) > 0) {
                Request req;
                it->second->get_cgi_response(it->second->fdOut, it->second->cgi_output);
                it->second->close_cgi();
                it->second->cgi_running = false;
                it->second->cgi_pid = -1;
                it->second->setResponse(parseCgiOutput(it->second->cgi_output, req));
            }
            else if (std::difftime(std::time(NULL), it->second->startTime) > 10) {
            Response res;
                kill(it->second->cgi_pid, SIGKILL);
                it->second->close_cgi();
                it->second->cgi_running = false;
                it->second->cgi_pid = -1;
                it->second->setResponse(res.ErrorResponse(504, this->_server.error_pages));
            }
            _writeEvent(this->_epollFd, it->first);
    }

        if (should_close) {
            int fd_to_close = it->first;
            _closeCon(fd_to_close);
            it = _Clients.begin();
        } else {
            ++it;
        }
    }
}

bool Server::_isNewClient(int FdClient){
	if (std::find(_listfd.begin(), _listfd.end(), FdClient) != _listfd.end())
        return true;
	return false;
}

int Server::_setNonBlocking(int fd){
    int f = fcntl(fd, F_GETFL, 0);
    if (f == -1)
        return -1;
    return fcntl(fd, F_SETFL, f | O_NONBLOCK);
}

void InfoSocket::setSocket(int fd, uint16_t port, uint32_t ip, int family){
	
	this->_fd = fd;
    this->port = port;
    this->ip = ip;
    this->addr_len = sizeof(sockaddr_in);

	this->addr.sin_family = family;
    this->addr.sin_port = htons(this->port);
    this->addr.sin_addr.s_addr = htonl(this->ip);

	this->sock_event.events = EPOLLIN;
    this->sock_event.data.fd = this->_fd;
}

int InfoSocket::getFd() const{return _fd;}
int InfoSocket::setFd(int fd){ this->_fd = fd; return this->_fd < 0;}
epoll_event& InfoSocket::getSockEvent(){ return sock_event;};
socklen_t& InfoSocket::getsocklen(){ return addr_len; }
sockaddr_in& InfoSocket::getSockaddr(){ return addr; }
