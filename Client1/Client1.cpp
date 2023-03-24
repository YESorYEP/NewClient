﻿#include <iostream>
#include "Connection.h"//Класс в который обернуты сокеты
#include <fstream>
#include <vector>
#include <string>
int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        std::cout << "no flags argc!" << std::endl;
        std::cout << "Usage: Client.exe <ip> <portTCP> <portUDP> <Filename> <Timeout>" << std::endl;
        exit(1);
    }
    Connection* con = new Connection(argv[5]);//создаем соединение и передаем в него таймаут
    con->InitClient(argv[1], atoi(argv[2]), atoi(argv[3]));//подключаемся к серверу(передаем ip адрес,tcp порт и udp порт
    std::vector<std::string> vecline;
    std::string line;
    std::ifstream in(argv[4]); // открываем файл для чтения
    if (in.is_open())
    {
        int ch = 0;
        while ((ch = in.get()) != EOF)
        {
            line += ch;
            if (line.size() == 242)
            {
                vecline.push_back(line);
                line = "";
            }
        }
        if (line != "")
            vecline.push_back(line);
    }
    in.close();     // закрываем файл
    std::string path = argv[4];
    std::string filename = path.substr(path.rfind('\\') + 1);//выделяем имя файла из полного пути к файлу
    con->Send(argv[3]);//отправляем на сервер udp порт через tcp
    con->Send(filename);//передаем на сервер имя файла
    con->Send(std::to_string(vecline.size()));//передаем на сервер кол-во строк
    std::cout << "Starting transmission" << std::endl;//начинаем передачу
    std::string id = "";
    for (int i = 0; i < vecline.size(); i++)
    {
        id = "Begin";
        if (i / 10.0 < 1)
            id += "00000";
        else if ((i / 100.0 < 1) && (i / 10.0 >= 1))
            id += "0000";
        else if ((i / 1000.0 < 1) && (i / 100.0 >= 1))
            id += "000";
        else if ((i / 10000.0 < 1) && (i / 1000.0 >= 1))
            id += "00";
        else if ((i / 100000.0 < 1) && (i / 10000.0 >= 1))
            id += "00";
        else if ((i / 1000000.0 < 1) && (i / 100000.0 >= 1))
            id += "00";
        else
            id += "0";
        id += std::to_string(i) + "End";
        id += vecline[i]; //обертываем udp пакет служебными данными id формата Begin000001EndData
        con->SendUDP(id);//отправляем udp пакет на сервер
        //Sleep(1000);
        con->Receive();//получаем подтверждение от сервера через tcp
        std::cout << "i" << i << std::endl;
        if ((i != 0) && (i != atoi(con->GetBuffer())))//в случае если пакеты
            i--;//потерялись по дороге то уменьшаем i и по циклу состоится
        id = "";//повторная передача
    }
    con->ClientClose();
    con->~Connection();
    std::cout << "Done!\n";
}