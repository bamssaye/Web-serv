#include "../inc/server.hpp"
// #include "../inc/client.hpp"

Server::Server (ServerConfig& servers) : InfoSocket(), _oP(1), _server(servers){
	try
	{
        this->initServer();
        this->runServer();
        // while(1);
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
    if((this->_epollFd = epoll_create1(EPOLL_CLOEXEC)) == -1){
        throw std::runtime_error("epoll_create1 failed");}
    for (std::map<unsigned long, int>::iterator it = _server.listen.begin();it != _server.listen.end(); ++it){
        unsigned long host = it->first ; int port = it->second; int i = 0;
        // int serveSize = _server.listen.size();
        InfoSocket sockets[2];
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw std::runtime_error("socket failed");
        this->_listfd.push_back(fd);
        sockets[i].setSocket(fd, port, host, AF_INET);
        // this->setSocket(fd, port, host, AF_INET);
        if (this->_setNonBlocking(fd) == -1)
            throw std::runtime_error("setNonBlocking failed");
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &this->_oP, sizeof(this->_oP)) < 0)
            throw std::runtime_error("setsockopt failed");
        if (bind(fd, (sockaddr *)&sockets[i].getSockaddr(), sockets[i].getsocklen()) < 0)
            throw std::runtime_error("bind failed");
        if (listen(fd, SOMAXCONN) < 0)
            throw std::runtime_error("listen failed");
        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &sockets[i].getSockEvent()) == -1)
            throw std::runtime_error("epoll_ctl failed");
    }
    // if (this->setFd(socket(AF_INET, SOCK_STREAM, 0)) < 0)
    //     throw std::runtime_error("epoll_create1 failed");
    
    // this->setSocket(this->getFd(), _server.port, _server.host, AF_INET);
    // if (this->_setNonBlocking(this->getFd()) == -1)
    //     throw std::runtime_error("epoll_create1 failed");

    // if (setsockopt(this->getFd(), SOL_SOCKET, SO_REUSEADDR, &this->_oP, sizeof(this->_oP)) < 0)
    //     throw std::runtime_error("epoll_create1 failed");
    // if (bind(this->getFd(), (sockaddr *)&this->addr, this->addr_len) < 0)
    //     throw std::runtime_error("epoll_create1 failed");
    // if (listen(this->getFd(), SOMAXCONN) < 0)
    //     throw std::runtime_error("epoll_create1 failed");
    // if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, this->getFd(), &this->sock_event) == -1)
    //     throw std::runtime_error("epoll_create1 failed");
    std::cout << "Server initialized." << std::endl;
}
template <typename T>
std::string Server::_toString(T value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
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
            _Clients[cliFd].setRequest(this->_epollFd);
        } 
    }
    else if (bytes == 0){
        this->_closeCon(cliFd);   
    }
}
void Server::_ClientWrite(int cliFd){
    std::cerr << "READ file  : " << std::endl;
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
void Server::_MsgErr(std::string m){ std::cerr << m << std::endl;}
void Server::_closeCon(int FdClient){
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, FdClient, NULL);
        close(FdClient);
        _Clients.erase(FdClient);
        this->_Msg("Connection closed: Client id = " + this->_toString(FdClient));
}
bool Server::_isNewClient(int FdClient){ //to check
	if (std::find(_listfd.begin(), _listfd.end(), FdClient) != _listfd.end())
        return true;
    // if(this->getFd() == FdClient)
	// 	return true;
	return false;
}
void Server::_AcceptCon(int FdServer){
	sockaddr_in cliAdd;
	socklen_t cliAddLen = sizeof(cliAdd);

	while (true){
		int cliFd = accept(FdServer, (sockaddr*)&cliAdd, &cliAddLen);
		if (cliFd == -1){
			if (errno == EAGAIN || errno == EWOULDBLOCK){break;}
			std::string msg = "accept failed: ";
			msg.append(strerror(errno));
			this->_MsgErr(msg);
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
		// store client data;
		_Clients[cliFd] = Client(cliFd, cliAdd); // sserver 0 cs have one server
	}

}


InfoSocket::InfoSocket():_fd(-1), port(0), ip(0){
    memset(&this->addr, 0, sizeof(this->addr));
    memset(&this->sock_event, 0, sizeof(this->sock_event));
}
InfoSocket::~InfoSocket(){}
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
