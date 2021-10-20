#include "server.h"

#include <sys/socket.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <ctime>
#include <cstdio>

#include "logger.h"

Server::Server(const uint16_t _port, int _socket_buffer_size, const char* output_file_path) : port(_port),
                                                                socket_buffer_size(_socket_buffer_size),
                                                                status(not_created){

    log.open (output_file_path, std::ofstream::out | std::ofstream::app);
    if (!log.is_open()){
        fprintf(stderr, "[Server] %s ERROR : %s\n", now_str().c_str(), "can not open log.txt");
        return;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0){
        status = socket_creat_error;
        fprintf(stderr, "[Server] %s ERROR : %s\n", now_str().c_str(), "can not create socket");
        return;
    }
//    fprintf(stdout, "[Server] %s : %s\n", now_str().c_str(), "socket created");

    server_adress.sin_family = AF_INET;
    server_adress.sin_port = htons(port);
    server_adress.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(server_socket, reinterpret_cast<struct sockaddr*>(&server_adress), sizeof(server_adress)) < 0){
        status = socket_bind_error;
        fprintf(stderr, "[Server] %s ERROR : %s\n", now_str().c_str(),"can not bind socket");
        return;
    }    
    fprintf(stdout, "[Server] %s : %s\n", now_str().c_str(), "socket binded");

    if (listen(server_socket, 5) < 0){ // todo: set backlog
        status = socket_listen_error;
        fprintf(stderr, "[Server] %s ERROR : %s\n", now_str().c_str(), "can not listen socket");
        return;
    }
//    fprintf(stdout, "[Server] %s : %s\n", now_str().c_str(), "socket listened");
    status = created;
    fprintf(stdout, "[Server] %s : server created no port: %i\n", now_str().c_str(), port);
}

Server::~Server()
{
    log.close();
}

int Server::run(){
    if (status != created){
        fprintf(stderr, "[Server] %s ERROR : %s\n", now_str().c_str(), "Server not created");
        return -1;
    }
    fprintf(stdout, "[Server] %s : %s\n", now_str().c_str(), "START SERVER");
    status = up;
    socklen_t size = sizeof(server_adress);
    while (status == up){
        int client_socket = accept(server_socket, reinterpret_cast<struct sockaddr*>(&server_adress), &size);
        if (client_socket < 0){
            if (status != stopped){
                fprintf(stderr, "[Server] %s ERROR : %s\n", now_str().c_str(), "can not accepting client");
            }
            continue;
        }
        Client *client = new Client(client_socket, socket_buffer_size);
        std::thread thr(&Server::clientLoop, this, client);
        clients.push_back(client);
        threads.push_back(std::move(thr));
        fprintf(stdout, "[Server] %s : NEW CONNECTION Client ID %i\n", now_str().c_str(), client_socket);
    }
    fprintf(stdout, "[Server] %s : %s\n", now_str().c_str(), "STOP SERVER");
    for (Client *client: clients){
        client->stop_receiving();
    }
    for (auto &thr: threads){
        thr.join();
    }
    for (Client *client: clients){
        delete client;
    }
    fprintf(stdout, "[Server] %s : %s\n", now_str().c_str(), "Bye");
    return 0;
}

void Server::clientLoop(Client *client){
    while (status == up) {
        std::string str = client->receive_string();
        if (str.length() > 0){
            log_mutex.lock();
            log << str;
            log.flush();
            log_mutex.unlock();
        } else {
            break;
        }
    }
}

void Server::stop()
{
    fprintf(stdout, "[Server] %s : %s\n", now_str().c_str(), "close server socket");
    status = stopped;
    close(server_socket);
}

int Server::getStatus()
{
    return status;
}

Server::Client::Client(int socket, size_t socket_buffer_size): socket(socket), socket_buffer_size(socket_buffer_size)
{
    fprintf(stdout, "[Server][Client ID %i] %s : %s\n", socket, now_str().c_str(), "connected");
}

Server::Client::~Client()
{
    fprintf(stdout, "[Server][Client ID %i] %s : %s\n", socket, now_str().c_str(), "close socket");
    close(socket);
}

std::string Server::Client::receive_string()
{
    std::string command;
    while (true) {
        size_t command_end = request_buffer.find('\n');
        if (command_end != std::string::npos){
            command = request_buffer.substr(0, command_end + 1);
            request_buffer.erase(0, command_end + 1);
            fprintf(stdout, "[Server][Client ID %i] %s : receved from client \"%s\"\n", socket, now_str().c_str(), CRton(command).c_str());
            break;
        }
        char buffer[socket_buffer_size];
        std::fill_n(buffer, socket_buffer_size, 0);
        int data_length = recv(socket, buffer, socket_buffer_size, 0);
        if (data_length == 0){
            fprintf(stdout, "[Server][Client ID %i] %s : %s\n", socket, now_str().c_str(), "reception stopped");
            break;
        }
        if (data_length == -1){
            fprintf(stdout, "[Server][Client ID %i] %s : %s\n", socket, now_str().c_str(), "client closed socket");
            break;
        }
        request_buffer += std::string(buffer, data_length);
    }
    return command;
}

void Server::Client::stop_receiving()
{
    fprintf(stdout, "[Server][Client ID %i] %s : %s\n", socket, now_str().c_str(), "stop receiving");
    shutdown(socket, SHUT_RD);
}
