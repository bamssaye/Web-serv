#include "../inc/InfoSocket.hpp"

InfoSocket::InfoSocket():_fd(-1), port(0), ip(0){
    memset(&this->addr, 0, sizeof(this->addr));
    memset(&this->sock_event, 0, sizeof(this->sock_event));
}
InfoSocket::~InfoSocket(){}


int InfoSocket::getFd() const{return _fd;}
int InfoSocket::setFd(int fd){ this->_fd = fd; return this->_fd < 0;}
epoll_event& InfoSocket::getSockEvent(){ return sock_event;};
socklen_t& InfoSocket::getsocklen(){ return addr_len; }
sockaddr_in& InfoSocket::getSockaddr(){ return addr; }


void InfoSocket::setSocket(int fd, uint16_t port, uint32_t ip, int ipVersion){
	
	this->_fd = fd;
    this->port = port;
    this->ip = ip;
    this->addr_len = sizeof(sockaddr_in);

	this->addr.sin_family = ipVersion;
    this->addr.sin_port = htons(this->port);
    this->addr.sin_addr.s_addr = htonl(this->ip);

	this->sock_event.events = EPOLLIN;
    this->sock_event.data.fd = this->_fd;
}