#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include <fcntl.h>
#include <string.h>
#include <sstream>

int main() {
    std::ostringstream res;
    res << "HTTP/1.1 200 OK\r\n";// << code << " " << this->getCodeMessage(code) << "\r\n";
    res << "Content-Type: text/html\r\n";
    res << "Content-Length: 2 \r\n";
    res << "Connection: close\r\n\r\n";
    res << "OK";
    std::string ss = res.str();
    int server_fd = socket(AF_INET, SOCK_STREAM | O_NONBLOCK, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    int op = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    
     if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port 8080\n";
    while (true) {
        int client_fd = accept(server_fd, NULL, NULL);
        
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK){
                continue;
            }
            else{
            std::cout << "Client connected!\n";}
            close(client_fd);
        }else{
            // fcntl(client_fd, F_SETFL, O_NONBLOCK);
            char buf[2];
            int n = recv(client_fd, buf, sizeof(buf), 0);
            // fcntl(client_fd, F_SETFL, O_NONBLOCK);
            if (n == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "client_fd is NON-blocking\n";
                } else {
                perror("recv");
                }
            } else if (n == 0) {
                std::cout << "Client closed connection\n";
            } else {
                std::cout << "Read " << n << " bytes: " << std::string(buf, n) << "\n";
            }
            close(client_fd);
        }
        
    }
    // close(client_fd);
    close(server_fd);
}
