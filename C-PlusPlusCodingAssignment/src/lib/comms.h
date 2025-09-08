#ifndef COMMS_H
#define COMMS_H

#include <string>

// Connect to server
// Params : 
//      host:- IP address of the host.
//      port:- Linstning port of the server
int connect_to_server(const std::string &host, int port);

// Create a listening server socket
// Params :
//      port:- Linstning port of the server.
//      backlog:- Backlog queue count for the accepting the connections.
int create_server_socket(int port, int backlog = 10);

// Accept a client connection
// Params:
//      server_fd :- Listning Socket fd
int accept_client(int server_fd);

// Send a message over TCP socket
// Params:- 
//      sock_fd :- Socket over which message will be sent.
//      msg :- Actual message
bool send_message(int sock_fd, const std::string &msg);

// Receive a message from TCP socket
// Params:- 
//      sock_fd :- Socket over which message will be received.
std::string recv_message(int sock_fd);

// Set a socket timeout in seconds
// Params:- 
//      scok_fd :- Socket fd for which timeout will be set.
//      seconds :- timeout in seconds.
bool set_socket_timeout(int sock_fd, int seconds);

#endif