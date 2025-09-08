#include "comms.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

// Connect to server at host:port
int connect_to_server(const std::string &host, int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); return -1; }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        perror("inet_pton"); close(sock_fd); return -1;
    }

    if (connect(sock_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); close(sock_fd); return -1;
    }

    set_socket_timeout(sock_fd, 60);
    return sock_fd;
}

// Create a listening server socket
int create_server_socket(int port, int backlog) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { 
        perror("socket"); 
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return -1;
    }

    if (listen(server_fd, backlog) < 0) {
        perror("listen"); close(server_fd); return -1;
    }

    return server_fd;
}

// Accept a client connection
int accept_client(int server_fd) {
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) 
        return -1;
    set_socket_timeout(client_fd, 60);
    return client_fd;
}

// Send a message over TCP socket
bool send_message(int sock_fd, const std::string &msg) {
    uint32_t len = htonl(msg.size());
    if (send(sock_fd, &len, sizeof(len), 0) != sizeof(len)) 
        return false;
    if (send(sock_fd, msg.c_str(), msg.size(), 0) != (ssize_t)msg.size()) 
        return false;
    return true;
}

// Receive a message from TCP socket
std::string recv_message(int sock_fd) {
    uint32_t len = 0;
    ssize_t r = recv(sock_fd, &len, sizeof(len), MSG_WAITALL);
    if (r <= 0) 
        return "";
    len = ntohl(len);

    std::string buffer(len, '\0');
    r = recv(sock_fd, buffer.data(), len, MSG_WAITALL);
    if (r <= 0) 
        return "";

    return buffer;
}

// Set a socket timeout in seconds
bool set_socket_timeout(int sock_fd, int seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt(SO_RCVTIMEO) failed");
        return false;
    }
    return true;
}

