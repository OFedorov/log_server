#include <iostream>
#include <cstdint>
#include <csignal>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "server.h"
#include "logger.h"


static Server * signal_object;
extern "C" void signal_handler(int signum) {signal_object->stop();}

int main(int argc, char** argv){
    if (argc < 2){
        fprintf(stderr, "[MAIN] %s ERROR : %s\n", now_str().c_str(), "port number not specified");
        return -1;
    }
    uint16_t port;
    std::string port_str = argv[1];
    if (!std::all_of(port_str.begin(), port_str.end(), [](unsigned char c){ return std::isdigit(c); })){
        fprintf(stderr, "[MAIN] %s ERROR : %s%s\n", now_str().c_str(), "Invalid port number: ", argv[1]);
        return -1;
    }
    port = std::stoi(port_str);

    Server server(port);
    if (server.getStatus() != Server::created){
        fprintf(stderr, "[MAIN] %s ERROR : %s\n", now_str().c_str(), "failed to create server");
        return -1;
    }
    signal_object = &server;
    signal(SIGINT, signal_handler);

    if (server.run() < 0){
        fprintf(stderr, "[MAIN] %s ERROR : %s\n", now_str().c_str(), "failed to run server");
        return -1;
    }
    return 0;
}
