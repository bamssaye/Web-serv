#pragma once

#include "webserv.h"


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
	void			setSocket(int fd, uint16_t port, uint32_t ip, int ipVersion);
	int 			setFd(int fd);
	int 			getFd() const;
	sockaddr_in&	getSockaddr();
	socklen_t&		getsocklen();
	epoll_event&	getSockEvent();
};