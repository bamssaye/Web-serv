#pragma once
#include "webserv.h"
#include "Parser.hpp"
#include "client.hpp"

class InfoSocket{

protected:
	int				_fd;
	sockaddr_in		addr;
	socklen_t		addr_len;
	epoll_event		sock_event;
	uint16_t		port;
	uint32_t		ip;

public:
	InfoSocket();
	~InfoSocket();
	//
	void setSocket(int fd, uint16_t port, uint32_t ip, int family);
	//
	int getFd() const;
	
	//
	int setFd(int fd);
	// InfoSocket();
	
};
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

///////

class Server : public InfoSocket {//}, public StatusCode {

	std::map <int, Client>	_Clients;
	epoll_event					_events[MAX_EVENTS];
	int 						_epollFd;
	int							_oP;
	std::vector<ServerConfig>&	_servers;
	int							_setNonBlocking(int fd);
	void						_Msg(std::string m);
	void						_MsgErr(std::string m);
	void						_handleEvent(const epoll_event& ev);
	void						_closeCon(int FdClient);
	bool						_isNewClient(int FdClient);
	void						_AcceptCon(int FdServer);
	void						_ClientRead(int cliFd);
	void						_ClientWrite(int cliFd);
public:
	Server (std::vector<ServerConfig>& servers);
	void initServer();
	void runServer();
	
	//
		
};
void Server::_MsgErr(std::string m){ std::cerr << m << std::endl;}
void Server::_closeCon(int FdClient){
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, FdClient, NULL);
        close(FdClient);
        // _Clients.erase(FdClient);
        this->_Msg("Connection closed: Client id = " + FdClient);
}
bool Server::_isNewClient(int FdClient){ //to check
	if(this->getFd() == FdClient)
		return true;
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