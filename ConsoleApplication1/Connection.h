#pragma once
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <WinSock.h>
#include <fcntl.h>
#pragma comment (lib,"WS2_32.lib")
#pragma warning(disable : 4996)
#include <vector>
class Connection
{
public:
    Connection(const char* timeout);
    int InitServer(const char* address, int port);
    int ServerClose();
    int Send(std::string line);
    int Receive();
    int SendUDP(std::string line);
    int ReceiveUDP();
    int InitClient(const char* address, int port, int udpport);
    int ClientClose();
    void bzero(char* buf, int l);
    void error(const char* msg);
    int SendServer(std::string line);
    int ReceiveServer();
    int ReceiveServerUDP();
    int SendServerUDP();
    char* GetBuffer();
    std::vector<std::string> GetVec();
    int Block(bool block);
    int BlockServer(bool block);
    int InitServerUDP(const char* address, int port);
    virtual ~Connection();
protected:
private:
    int sockfd, newsockfd, udpsocket;
    int buflen;
    char* buffer;
    struct sockaddr_in serv_addr, cli_addr, serv_addr_udp, cli_addr_udp;
    int clilen;
    int servlen;
    int n;
    std::string strFile, strCli;
    std::vector<std::string> vecline, veccli;
    struct timeval timeout;
};