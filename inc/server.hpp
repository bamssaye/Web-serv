#pragma once
/////////////////////
#include "webserv.h"
#include "Parser.hpp"
#include "client.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "InfoSocket.hpp"
//////////////////////

// class InfoSocket;
///////
class Server {

	std::map <int, Client*>		_Clients;
	int							_cliCount;
	epoll_event					_events[MAX_EVENTS];
	std::map <int, InfoSocket*>	_sockets;
	int 						_epollFd;
	int							_oP;
	ServerConfig				_server;
	std::vector<int>			_listfd;
	void						_Msg(std::string m);
	void						_MsgErr(std::string m);
	void						_handleEvent(const epoll_event& ev);
	void						_closeCon(int FdClient);
	bool						_isNewClient(int FdClient);
	void						_AcceptCon(int eventFD);
	///
	void						_ClientRead(int cliFd);
	void						_ClientWrite(int cliFd);

	void 						_ReadContent(char *buf, ssize_t byRead, int cliFd);
	///
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
	static int					getsig;
};

