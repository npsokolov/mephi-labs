#include "tcp_connect.h"
#include "byte_tools.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <limits>
#include <utility>
#include <iostream>

const std::string &TcpConnect::GetIp() const
{
    return ip_;
}

int TcpConnect::GetPort() const
{
    return port_;
}

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout) : ip_(ip), port_(port), connectTimeout_(connectTimeout), readTimeout_(readTimeout) {}

void TcpConnect::EstablishConnection()
{
    // creating a reciever socket
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0)
    {
        throw std::runtime_error("socket");
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(connectTimeout_).count();

    // setting up non block mode
    if (fcntl(sock_, F_SETFL, O_NONBLOCK) < 0)
    {
        throw std::runtime_error("fcntl");
    }

    struct sockaddr_in peer_addr;
    std::memset(&peer_addr, 0, sizeof(peer_addr));
    socklen_t peer_addr_size = sizeof(peer_addr);
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port_);

    // translating ip address
    if (inet_pton(AF_INET, ip_.c_str(), &peer_addr.sin_addr) < 0)
    {
        throw std::runtime_error("Error in IP translation to special numeric format");
    }

    // connecting to the peer
    if (connect(sock_, (sockaddr *)&peer_addr, peer_addr_size) < 0 && errno != EINPROGRESS)
    {
        throw std::runtime_error("Cannot connect to the server");
    }

    fd_set set_of_conditions;
    FD_ZERO(&set_of_conditions);
    FD_SET(sock_, &set_of_conditions);

    // waits some time to connect
    int select_checker = select(sock_ + 1, NULL, &set_of_conditions, NULL, &timeout);
    if (select_checker < 0)
    {
        throw std::runtime_error("select");
    }
    else if (select_checker == 0)
    {
        throw std::runtime_error("Connection time out");
    }

    // checks if socket is ok
    int connect_error = 0;
    socklen_t error_size = sizeof(connect_error);
    if (getsockopt(sock_, SOL_SOCKET, SO_ERROR, &connect_error, &error_size) < 0)
    {
        throw std::runtime_error("getsockopt");
    }
    if (connect_error != 0)
    {
        throw std::runtime_error("Cannot connect to the server");
    }
}

void TcpConnect::SendData(const std::string &data) const
{
    // sending data
    int time_to_read = std::chrono::duration_cast<std::chrono::milliseconds>(readTimeout_).count();
    int n = data.size();
    if (n == 0)
    {
        return;
    }
    char to_send[n];
    for (int i = 0; i < n; i++)
    {
        to_send[i] = data[i];
    }
    struct pollfd poll_conditions;
    poll_conditions.fd = sock_;
    poll_conditions.events = POLLOUT;
    int p = poll(&poll_conditions, 1, time_to_read);
    if (send(sock_, to_send, n, 0) < 0)
    {
        throw std::runtime_error("send");
    }
}

std::string TcpConnect::ReceiveData(size_t bufferSize) const
{
    int time_to_read = std::chrono::duration_cast<std::chrono::milliseconds>(readTimeout_).count();
    struct pollfd poll_conditions;
    poll_conditions.fd = sock_;
    poll_conditions.events = POLLIN;
    int p = poll(&poll_conditions, 1, time_to_read);
    if (p < 0)
    {
        throw std::runtime_error("poll < 0");
    }
    else if (p == 0)
    {
        throw std::runtime_error("poll didn't wait so long");
    }
    else
    {
        if (poll_conditions.revents & POLLIN)
        {
            if (bufferSize == 0)
            {
                char readed[4];
                int recv_return_statement = recv(sock_, readed, sizeof(readed), 0);
                if (recv_return_statement < 0)
                {
                    throw std::runtime_error("recv < 0 while receiving size");
                }
                else if (recv_return_statement == 0)
                {
                    return std::string();
                }

                std::string size;
                for (int i = 0; i < 4; ++i)
                {
                    size += readed[i];
                }

                bufferSize = BytesToInt(size);

                if (bufferSize > ((1 << 16) * 2 - 1))
                {
                    throw std::runtime_error("Cannot receive so much info");
                }
                char buffer[bufferSize];
                std::string data;

                size_t to_be_read = bufferSize;

                auto from = std::chrono::steady_clock::now();

                do
                {
                    struct pollfd poll_conditions;
                    poll_conditions.fd = sock_;
                    poll_conditions.events = POLLIN;
                    int p = poll(&poll_conditions, 1, time_to_read);
                    if (p == 0)
                    {
                        throw std::runtime_error("poll didn't wait so long during receiving huge data");
                    }
                    auto now = std::chrono::steady_clock::now() - from;
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(now) > readTimeout_)
                    {
                        throw std::runtime_error("Time has expired during receiving");
                    }
                    recv_return_statement = recv(sock_, buffer, to_be_read, 0);
                    to_be_read -= recv_return_statement;
                    if (recv_return_statement <= 0)
                    {
                        throw std::runtime_error("recv <= 0 during receiving huge data");
                    }
                    for (int i = 0; i < recv_return_statement; ++i)
                    {
                        data.push_back((unsigned char)buffer[i]);
                    }
                } while (to_be_read > 0);
                return data;
            }
            else
            {
                char readed[bufferSize];
                int recv_return_statement = recv(sock_, readed, sizeof(readed), 0);
                if (recv_return_statement < 0)
                {
                    throw std::runtime_error("recv < 0 while receiving size");
                }
                std::string data;
                for (int i = 0; i < bufferSize; ++i)
                {
                    data += readed[i];
                }
                return data;
            }
        }
        else
        {
            throw std::runtime_error("something went wrong with POLLIN status");
        }
    }
}

void TcpConnect::CloseConnection()
{
    if (sock_ >= 0)
    {
        close(sock_);
    }
}

TcpConnect::~TcpConnect()
{
    if (sock_ >= 0)
    {
        close(sock_);
    }
}
