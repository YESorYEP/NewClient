#include "Connection.h"
using namespace std;

void Connection::bzero(char* buf, int l)
{
    memset(buf, 0, l);
}
void Connection::error(const char* msg)
{
    int err = WSAGetLastError();
    perror(msg);
    cout << err << endl;
    WSACleanup();
    cin.ignore();
    exit(1);
}
Connection::Connection(const char* timeout_)
{
    buflen = 256;
    buffer = new char[buflen];
    strFile = "";
    strCli = "";
    timeout.tv_sec = 0;
    if (timeout_ != "")
        timeout.tv_usec = atoi(timeout_);
    else
        timeout.tv_usec = 500;
}
int Connection::InitServer(const char* address, int port_)
{
    WSADATA ws = { 0 };
    if (WSAStartup(MAKEWORD(2, 2), &ws) == 0)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0)
            error("ERROR opening socket");
        bzero((char*)&cli_addr, sizeof(cli_addr));
        memset(serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port_);
        string str = "";
        int i = 0;
        while (address[i] != '.')
            str += address[i++];
        i++;
        //cout << str<< endl;
        serv_addr.sin_addr.S_un.S_un_b.s_b1 = atoi(str.c_str());
        str = "";
        while (address[i] != '.')
            str += address[i++];
        i++;
        serv_addr.sin_addr.S_un.S_un_b.s_b2 = atoi(str.c_str());
        //cout << str << endl;
        str = "";
        while (address[i] != '.')
            str += address[i++];
        i++;
        serv_addr.sin_addr.S_un.S_un_b.s_b3 = atoi(str.c_str());
        
        str = "";
        while (i != strlen(address))
            str += address[i++];
        
        serv_addr.sin_addr.S_un.S_un_b.s_b4 = atoi(str.c_str());

        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR on binding");
        if (listen(sockfd, SOMAXCONN) < 0)
            error("ERROR on listen");
        cout << "Waiting client" << endl;
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        cout << "Client connected" << endl;
        if (newsockfd < 0)
            error("ERROR on accept");
        
        if (setsockopt(newsockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof timeout) < 0)
            error("setsockopt failed\n");
    }
    else
        WSAGetLastError();
    return 0;
}

int Connection::InitServerUDP(const char* address, int port)
{
    udpsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpsocket < 0)
        error("ERROR opening socket");
    serv_addr_udp.sin_family = AF_INET;
    serv_addr_udp.sin_port = htons(port);
    string str = "";
    int i = 0;
    while (address[i] != '.')
        str += address[i++];
    i++;
    serv_addr_udp.sin_addr.S_un.S_un_b.s_b1 = atoi(str.c_str());//задаем октеты адреса
    str = "";
    while (address[i] != '.')
        str += address[i++];
    i++;
    serv_addr_udp.sin_addr.S_un.S_un_b.s_b2 = atoi(str.c_str());//задаем октеты адреса
    str = "";
    while (address[i] != '.')
        str += address[i++];
    i++;
    serv_addr_udp.sin_addr.S_un.S_un_b.s_b3 = atoi(str.c_str());//задаем октеты адреса
    str = "";
    while (i != strlen(address))
        str += address[i++];
    serv_addr_udp.sin_addr.S_un.S_un_b.s_b4 = atoi(str.c_str());//задаем октеты адреса
    if (bind(udpsocket, (struct sockaddr*)&serv_addr_udp, sizeof(serv_addr_udp)) < 0)
        error("ERROR on binding");
    cout << "Udp Ready" << endl;
    return 0;
}
char* Connection::GetBuffer()
{
    return buffer;
}
vector<string> Connection::GetVec()
{
    return vecline;
}
int Connection::ServerClose()
{
    shutdown(newsockfd, 0);
    shutdown(sockfd, 0);
    shutdown(udpsocket, 0);
    closesocket(newsockfd);
    closesocket(sockfd);
    closesocket(udpsocket);
    WSACleanup();
    return 0;
}
int Connection::SendServer(string line)
{
    memset(buffer, 0, buflen);
    n = send(newsockfd, line.c_str(), buflen, 0);
    
    if (n < 0)
        error("ERROR on send");
    return n;
}
int Connection::Send(string line)
{
    n = send(sockfd, line.c_str(), buflen, 0);
    if (n < 0)
        error("ERROR on send");
    return n;
}
int Connection::SendUDP(string line)
{
    n = sendto(udpsocket, line.c_str(), buflen, 0, (struct sockaddr*)&serv_addr_udp, sizeof serv_addr_udp);
    if (n < 0)
        error("ERROR on send udp");
    
    return n;
}
int Connection::ReceiveServer()
{
    memset(buffer, 0, buflen);
    n = recv(newsockfd, buffer, buflen, 0);
    if (n < 0)
        error("ERROR on read");
    return n;
}
int Connection::ReceiveServerUDP()
{
    memset(buffer, 0, buflen);
    int slen = sizeof(sockaddr_in);
    n = recvfrom(udpsocket, buffer, buflen, 0, (struct sockaddr*)&cli_addr_udp, &slen);
    
    if (n < 0)
        error("ERROR on read udp");
    for (int i = 0; i < strlen(buffer); i++)
        strFile += buffer[i];
    if (vecline.size() != 0 && strFile == vecline[vecline.size() - 1])
        return n;//проверяем не повторилась ли передача udp пакета
    vecline.push_back(strFile);
  
    strFile = "";
    return n;
}
int Connection::Receive()
{
    memset(buffer, 0, buflen);
    n = recv(sockfd, buffer, buflen, 0);
    
    if (n < 0)
        error("ERROR on read");
    for (int i = 0; i < strlen(buffer); i++)
        strCli += buffer[i];
    if (veccli.size() != 0 && strCli == veccli[veccli.size() - 1])
        return n;//проверяем не повторилась ли передача tcp пакета
    veccli.push_back(strCli);
    strCli = "";
    return n;
}
int Connection::Block(bool block)
{
    u_long argp = block ? 1 : 0;
    ioctlsocket(sockfd, FIONBIO, &argp);
    return n;
}
int Connection::BlockServer(bool block)
{
    u_long argp = block ? 1 : 0;
    ioctlsocket(newsockfd, FIONBIO, &argp);
    return n;
}
Connection::~Connection()
{
    delete[] buffer;
}
int Connection::InitClient(const char* address_, int port_, int udpport)
{
    WSADATA ws = { 0 };
    if (WSAStartup(MAKEWORD(2, 2), &ws) == 0)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0)
            error("ERROR opening socket");
        bzero((char*)&serv_addr, sizeof(serv_addr));
        bzero((char*)&cli_addr, sizeof(cli_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(address_);
        serv_addr.sin_port = htons(port_);
        servlen = sizeof(serv_addr);
        n = connect(sockfd, (struct sockaddr*)&serv_addr, servlen);
        if (n < 0)
            error("ERROR on connect");
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof timeout) < 0)
            error("setsockopt failed");
        udpsocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpsocket < 0)
            error("ERROR opening udp socket");
        memset((char*)&serv_addr_udp, 0, sizeof(serv_addr_udp));
        serv_addr_udp.sin_family = AF_INET;
        serv_addr_udp.sin_port = htons(udpport);
        serv_addr_udp.sin_addr.S_un.S_addr = inet_addr(address_);
        
    }
    else
        WSAGetLastError();
    return 0;
}
int Connection::ClientClose()
{
    shutdown(udpsocket, 0);
    shutdown(sockfd, 0);
    closesocket(udpsocket);
    closesocket(sockfd);
    WSACleanup();
    return 0;
}