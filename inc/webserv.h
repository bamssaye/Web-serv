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
#include <ctime>
#include <limits>
#include <signal.h>

#define MAX_EVENTS  10 // MAX event 
#define BUFFER_SIZE 8192 // 8kb
#define MAX_SIZE    524288 // 500kb
#define CGI_BUFSIZE 4096 

struct FormPart {
	std::string name;
	std::string filename;
	std::string contentType;
	std::string content;
};

struct LocationConfig {
    public:
    std::string              path;
    std::string              root;
    std::string              index;
    bool                     autoindex;
    std::vector<std::string> allowed_methods;
    int                      return_code;
    std::string              return_url;
    std::string              upload_path;
    std::map<std::string, std::string> cgi;
    std::map<std::string, std::string> cgi_params;

    LocationConfig();
    LocationConfig(const LocationConfig& other);
    LocationConfig& operator=(const LocationConfig& other);
};

struct ServerConfig {
    public:
    std::vector<std::pair<unsigned long, unsigned short> >  listen;
    std::vector<std::string>                  host_str;
    long                         client_max_body_size;
    std::map<int, std::string>   error_pages;
    std::vector<LocationConfig>  locations;

    ServerConfig();
    ServerConfig(const ServerConfig& other);
    ServerConfig& operator=(const ServerConfig& other);
};

template <typename T>
std::string _toString(T value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
};

#endif