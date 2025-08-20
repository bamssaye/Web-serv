#pragma once
#include "webserv.h"
#include "Parser.hpp"
#include "client.hpp"
// class Client;
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
	sockaddr_in& getSockaddr(){
		return addr;
	}
	socklen_t& getsocklen(){
		return addr_len;
	}
	epoll_event& getSockEvent() {
		return sock_event;
	}
	
	// InfoSocket();
	
};
///////
class Server : public InfoSocket {//}, public StatusCode {

	std::map <int, Client>		_Clients;
	epoll_event					_events[MAX_EVENTS];
	int 						_CliCount;
	int 						_epollFd;
	int							_oP;
	ServerConfig				_server;
	std::vector<int>			_listfd;
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
	Server (ServerConfig& servers);
	void initServer();
	void runServer();
	
	//
	template <typename T> std::string _toString(T value);
	std::vector<int> getListFd;

		
};

