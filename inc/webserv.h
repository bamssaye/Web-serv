#ifndef WEBSERV_HPP
#define WEBSERV_HPP


#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <cctype>
#include <cstdlib>

#include <algorithm>
#include <vector>
#include <map>

#include <stdexcept>
#include <exception>

#include <errno.h>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096


struct FormPart {
	std::string name;
	std::string filename;
	std::string contentType;
	std::string content;
};

enum Methods
{
	GET,
	DELETE,
	POST,
};

#endif