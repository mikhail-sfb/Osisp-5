// client.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>э
#include <clocale>


#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "54000"

void ReceiveMessages(SOCKET connectSocket) {
    char recvbuf[512];
    int recvbuflen = 512;

    while (true) {
        int result = recv(connectSocket, recvbuf, recvbuflen, 0);
        if (result > 0) {
            recvbuf[result] = '\0';
            std::cout << "\nПолучено сообщение: " << recvbuf << "\n> ";
            std::cout.flush();
        }
        else if (result == 0) {
            std::cout << "Соединение закрыто сервером.\n";
            break;
        }
        else {
            std::cout << "recv не удалось: " << WSAGetLastError() << "\n";
            break;
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    WSADATA wsaData;
    int result;
    #include <clocale>


    // Инициализация Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cout << "WSAStartup не удалось: " << result << "\n";
        return 1;
    }

    struct addrinfo* addrResult = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // IPv4 или IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP; // TCP

    // Получаем адрес информации
    result = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &addrResult);
    if (result != 0) {
        std::cout << "getaddrinfo не удалось: " << result << "\n";
        WSACleanup();
        return 1;
    }

    SOCKET connectSocket = INVALID_SOCKET;

    // Подключаемся к серверу
    for (ptr = addrResult; ptr != NULL; ptr = ptr->ai_next) {
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            std::cout << "socket не удалось: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 1;
        }

        result = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(addrResult);

    if (connectSocket == INVALID_SOCKET) {
        std::cout << "Не удалось подключиться к серверу.\n";
        WSACleanup();
        return 1;
    }

    std::cout << "Подключено к серверу.\n";

    // Запускаем поток для получения сообщений
    std::thread recvThread(ReceiveMessages, connectSocket);
    recvThread.detach();

    // Отправляем сообщения
    std::string message;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);

        if (message == "/exit") {
            break;
        }

        int sendResult = send(connectSocket, message.c_str(), (int)message.length(), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cout << "send не удалось: " << WSAGetLastError() << "\n";
            break;
        }
    }

    // Закрываем соединение
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}
