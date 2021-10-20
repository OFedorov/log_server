#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <netinet/in.h>
#include <fstream>

class Server{
public:
    Server(const uint16_t _port, int _socket_buffer_size = 4096, const char* output_file_path = "log.txt");
    ~Server();

    int run();
    void stop();
    int getStatus();

    enum {not_created,
          created,
          up,
          stopped,
          socket_creat_error,
          socket_bind_error,
          socket_listen_error};

private:
    class Client;
    uint16_t port;
    int server_socket;
    struct sockaddr_in server_adress;
    int status;
    size_t socket_buffer_size;
    std::vector<std::thread> threads;
    std::vector<Client*> clients;

    std::ofstream log;
    std::mutex log_mutex;

    void clientLoop(Client *client);
};

class Server::Client{
public:
    Client(int socket, size_t socket_buffer_size);
    ~Client();

    std::string receive_string();
    void stop_receiving();
private:
    int socket;
    size_t socket_buffer_size;
    std::string request_buffer;
};

#endif //SEVER_H
