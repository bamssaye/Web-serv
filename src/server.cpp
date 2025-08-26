#include "../inc/server.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"
#include "../inc/CgiHandler.hpp"
#include "../inc/Library.hpp"
/// /////

Server::Server (ServerConfig& servers) : _cliCount(0), _oP(1), _server(servers){
    this->initServer();
}
Server::~Server(){
    for (std::map<int, Client*>::iterator it = _Clients.begin(); it != _Clients.end(); ++it)
        delete it->second;
    _Clients.clear();
    for (std::map<int, InfoSocket*>::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
        delete it->second;
    _sockets.clear();
}

/// ///// Set && Run Server

void Server::initServer(){
    if((this->_epollFd = epoll_create1(EPOLL_CLOEXEC)) == -1){
        throw std::runtime_error("epoll_create1 failed");}
    std::vector<std::pair<unsigned long, unsigned short> >::iterator it = _server.listen.begin();
    while (it != _server.listen.end()){
        unsigned long host = it->first ; int port = it->second; int i = 0;
        int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd < 0)
            throw std::runtime_error("socket failed");
        this->_listfd.push_back(fd);
        _sockets[i] = new InfoSocket();
        _sockets[i]->setSocket(fd, port, host, AF_INET);
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &this->_oP, sizeof(this->_oP)) < 0)
            throw std::runtime_error("setsockopt failed");
        if (bind(fd, (sockaddr *)&_sockets[i]->getSockaddr(), _sockets[i]->getsocklen()) < 0)
            throw std::runtime_error("bind failed");
        if (listen(fd, SOMAXCONN) < 0)
            throw std::runtime_error("listen failed");
        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &_sockets[i]->getSockEvent()) == -1)
            throw std::runtime_error("epoll_ctl failed");
        ++it;
        i++;
    }
    Library::printMsg("\n\t   Webserv/1.0\n\tServer initialized.");
}
void Server::runServer(){
    Library::printMsg("\tServer listening.\n\t" + std::string(20,'='));
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

/// ///// Handle Event
void Server::_handleEvent(const epoll_event& ev){
    int ev_fd = ev.data.fd;

    if (ev.events & (EPOLLERR | EPOLLHUP)){
        Library::printMsgErr("Client Error : " + _toString(ev_fd));
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

/// ///// Change Event MOD {EPOLLOUT, EPOLLIN}
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

/// ///// Read Request from Client {Request > 1MB convert to file Mode}
void Server::_ReadContent(char *buf, ssize_t byRead, int cliFd){
    
    if (_Clients[cliFd]->getRequHeaderCheck()){
        size_t len = _Clients[cliFd]->getContentLength();
        if (len > MAX_SIZE){
            _Clients[cliFd]->readlargeFileRequest(buf, byRead);
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
    }
}
/// /// Accepte New Connection {New Client}
void Server::_AcceptCon(int eventFD){
	sockaddr_in cliAdd;
	socklen_t cliAddLen = sizeof(cliAdd);

	while (true){
		int cliFd = accept(eventFD, (sockaddr*)&cliAdd, &cliAddLen);
		if (cliFd == -1){
			if (errno == EAGAIN || errno == EWOULDBLOCK){
                break;}
			std::string msg = "Accept failed: ";
			Library::printMsgErr(_toString(msg + strerror(errno)));
			break;
		}
		epoll_event cliEvent;
		cliEvent.data.fd = cliFd;
		cliEvent.events = EPOLLIN;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, cliFd, &cliEvent) == -1){
			Library::printMsgErr("Error: epoll_ctl ADD new client failed");
			close(cliFd);
			continue;
		}
        _cliCount++;
		_Clients[cliFd] = new Client(_server ,_cliCount);
        Library::printMsg("Client ID=[" + _toString(_cliCount) + "] Connected");
	}
}
void Server::_ClientRead(int cliFd){
    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    if (_Clients.find(cliFd) == _Clients.end())
        return;
    bytes = recv(cliFd, buffer, BUFFER_SIZE, 0);
    if (bytes == -1) {
        this->_closeCon(cliFd);
        return;
    }
    _ReadContent(buffer, bytes, cliFd);
    if (_Clients[cliFd]->requCheck && _Clients[cliFd]->requCheckcomp){
            _Clients[cliFd]->HttpRequest();
            _writeEvent(this->_epollFd, cliFd);
    } 
    if (bytes == 0){
        this->_closeCon(cliFd);   
    }
}

/// //// Write Response to Client (64kb -> ... -> Finish)
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



/// /// CLose Connection {Remove Client}
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
       Library::printMsg("Connection closed ID=[" + _toString(id) + "]");
    }
}

/// ///
void Server::checkTimeouts() {
    std::map<int, Client*>::iterator it = _Clients.begin();
    while (it != _Clients.end()){
        bool should_close = false;
        if (it->second->cgi_running){
            if (waitpid(it->second->cgi_pid, NULL, WNOHANG) > 0){
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
        if (should_close){
            int fd_to_close = it->first;
            _closeCon(fd_to_close);
            it = _Clients.begin();
        } 
        else{
            ++it;
        }
    }
}
bool Server::_isNewClient(int FdClient){
	if (std::find(_listfd.begin(), _listfd.end(), FdClient) != _listfd.end())
        return true;
	return false;
}


