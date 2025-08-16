#include "../inc/server.hpp"



Server::Server (std::vector<ServerConfig>& servers) : InfoSocket(), _oP(1), _servers(servers){
	try
	{
        this->initServer();
        while(1);
	}
	catch(const std::exception& e)
	{
        std::cerr << "\033[1;31mCritical Error: " << e.what() << "\033[0m" << std::endl;
		exit(EXIT_FAILURE);
	}
}
int Server::_setNonBlocking(int fd){
    int f = fcntl(fd, F_GETFL, 0);
    if (f == -1)
        return -1;
    return fcntl(fd, F_SETFL, f | O_NONBLOCK);
}
void Server::_Msg(std::string m){ std::cout << m << std::endl;}
void Server::initServer(){
    if((this->_epollFd = epoll_create1(EPOLL_CLOEXEC)) == -1)
        throw std::runtime_error("epoll_create1 failed");
    if (this->setFd(socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw std::runtime_error("epoll_create1 failed");
    this->setSocket(this->getFd(), _servers[0].port, _servers[0].host, AF_INET);
    if (this->_setNonBlocking(this->getFd()) == -1)
        throw std::runtime_error("epoll_create1 failed");
    if (setsockopt(this->getFd(), SOL_SOCKET, SO_REUSEADDR, &this->_oP, sizeof(this->_oP)) < 0)
        throw std::runtime_error("epoll_create1 failed");
    if (bind(this->getFd(), (sockaddr *)&this->addr, this->addr_len) < 0)
        throw std::runtime_error("epoll_create1 failed");
    if (listen(this->getFd(), SOMAXCONN) < 0)
        throw std::runtime_error("epoll_create1 failed");
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, this->getFd(), &this->sock_event) == -1)
        throw std::runtime_error("epoll_create1 failed");
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
    //    if (!nFds){
    //     //clean old client;
    //    }
       for (int i = 0; i < nFds ; i++){
        _handleEvent(_events[i]);
       }
    }
}
void Server::_handleEvent(const epoll_event& ev){
    int ev_fd = ev.data.fd;

    if (ev.events & (EPOLLERR | EPOLLHUP)){
        _MsgErr("Client Error on Fd : " + ev_fd);
        _closeCon(ev_fd);
        return;
    }
    if (_isNewClient(ev_fd)){
        if (ev.events & EPOLLIN){ _AcceptCon(ev_fd); }
    }
    else{
        if (ev.events & EPOLLIN){ _ClientRead(ev_fd);}
        else if (ev.events & EPOLLOUT){ _ClientWrite(ev_fd);}
    }
}
void Server::_ClientRead(int cliFd){
    char buffer[BUFFER_SIZE];
    ssize_t bytes;
    
    if (_Clients.find(cliFd) == _Clients.end())
        return;
    bytes = recv(cliFd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0){
        _Clients[cliFd].addBuffer(buffer, bytes);
        if (_Clients[cliFd].requCheck){
            _Clients[cliFd].setRequest();
        } 
    }
    else if (bytes == 0){
        this->_closeCon(cliFd);   
    }
}
void Server::_ClientWrite(int cliFd){
    if (_Clients.find(cliFd) == _Clients.end())
        return;
    while(_Clients[cliFd].dataPending()){
        ssize_t bySent = send(cliFd, _Clients[cliFd].getdataPending(), _Clients[cliFd].getSizePending(), MSG_NOSIGNAL);
        if (bySent > 0){
            _Clients[cliFd].dataSent(bySent);
        }
    }
    if (!_Clients[cliFd].getdataPending()) {
            epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = cliFd;
            epoll_ctl(this->_epollFd, EPOLL_CTL_MOD, cliFd, &event);
    }
}