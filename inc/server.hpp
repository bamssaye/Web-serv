#pragma once
/////////////////////
#include "webserv.h"
#include "Parser.hpp"
#include "client.hpp"
//////////////////////

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
	void			setSocket(int fd, uint16_t port, uint32_t ip, int family);
	int 			setFd(int fd);
	int 			getFd() const;
	sockaddr_in&	getSockaddr();
	socklen_t&		getsocklen();
	epoll_event&	getSockEvent();
};

///////
class Server : public InfoSocket {//}, public StatusCode {

	std::map <int, Client*>		_Clients;
	int							_cliCount;
	epoll_event					_events[MAX_EVENTS];
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
	void 						_writeEvent(int epollFd, int fd);
	void 						_readEvent(int epollFd, int fd);
public:
	Server (ServerConfig& servers);
	~Server();
	///
	void checkTimeouts();
	void 						initServer();
	void 						runServer();
	std::vector<int> 			getListFd;		
};

