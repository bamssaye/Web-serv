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

struct LocationConfig {
    std::string              path;
    std::string              root;
    std::string              index;
    std::string              cgi_pass;
    bool                     autoindex;
    std::vector<Methods>     allowed_methods;
    int                      return_code;
    std::string              return_url;
    std::string              upload_path;
    std::vector<std::string> cgi_extensions;

    LocationConfig();
};

struct ServerConfig {
    unsigned long                host;
    int                          port;
    std::string                  root;
    std::vector<std::string>     server_names;
    long                         client_max_body_size;
    std::map<int, std::string>   error_pages;
    // std::pair<std::vector<int>, std::string> error_pages;
    std::vector<LocationConfig>  locations;
    std::map<unsigned long, int> listen; // to check

    ServerConfig();
};

// template <typename T>
// std::string _toString(T value)
// {
//     std::ostringstream oss;
//     oss << value;
//     return oss.str();
// }
#endif