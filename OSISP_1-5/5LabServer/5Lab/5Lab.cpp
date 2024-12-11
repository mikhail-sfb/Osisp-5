#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT "54000"

class Server {
public:
    Server() : listenSocket(INVALID_SOCKET) {}

    int Start() {
        if (InitializeWinsock() != 0) {
            return 1;
        }

        if (SetupServerSocket() != 0) {
            Cleanup();
            return 1;
        }

        if (ListenForConnections() != 0) {
            Cleanup();
            return 1;
        }

        HandleClientConnections();
        Cleanup();
        return 0;
    }

private:
    SOCKET listenSocket;
    std::vector<SOCKET> activeClients;
    std::mutex clientsMutex;

    int InitializeWinsock() {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "Ошибка инициализации Winsock: " << result << "\n";
            return result;
        }
        return 0;
    }

    int SetupServerSocket() {
        struct addrinfo* addrInfo = nullptr, hints;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        int result = getaddrinfo(NULL, SERVER_PORT, &hints, &addrInfo);
        if (result != 0) {
            std::cerr << "Ошибка при получении адреса: " << result << "\n";
            return result;
        }

        listenSocket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << "Ошибка создания сокета: " << WSAGetLastError() << "\n";
            freeaddrinfo(addrInfo);
            return SOCKET_ERROR;
        }

        result = bind(listenSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);
        freeaddrinfo(addrInfo);
        if (result == SOCKET_ERROR) {
            std::cerr << "Ошибка привязки сокета: " << WSAGetLastError() << "\n";
            closesocket(listenSocket);
            return SOCKET_ERROR;
        }

        return 0;
    }

    int ListenForConnections() {
        int result = listen(listenSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) {
            std::cerr << "Ошибка при прослушивании сокета: " << WSAGetLastError() << "\n";
            closesocket(listenSocket);
            return SOCKET_ERROR;
        }

        std::cout << "Сервер запущен и готов принимать подключения...\n";
        return 0;
    }

    void HandleClientConnections() {
        while (true) {
            SOCKET clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Ошибка принятия подключения: " << WSAGetLastError() << "\n";
                continue;
            }

            std::cout << "Новый клиент подключился.\n";

            std::lock_guard<std::mutex> lock(clientsMutex);
            activeClients.push_back(clientSocket);

            std::thread clientThread(&Server::ClientHandler, this, clientSocket);
            clientThread.detach();
        }
    }

    void ClientHandler(SOCKET clientSocket) {
        char buffer[512];
        int bufferLength = sizeof(buffer);

        while (true) {
            int bytesReceived = recv(clientSocket, buffer, bufferLength, 0);
            if (bytesReceived > 0) {
                std::lock_guard<std::mutex> lock(clientsMutex);
                BroadcastMessage(buffer, bytesReceived, clientSocket);
            }
            else if (bytesReceived == 0) {
                std::cout << "Клиент отключился.\n";
                break;
            }
            else {
                std::cerr << "Ошибка получения данных: " << WSAGetLastError() << "\n";
                break;
            }
        }

        RemoveClient(clientSocket);
        closesocket(clientSocket);
    }

    void BroadcastMessage(const char* message, int length, SOCKET senderSocket) {
        for (SOCKET client : activeClients) {
            if (client != senderSocket) {
                send(client, message, length, 0);
            }
        }
    }

    void RemoveClient(SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        activeClients.erase(std::remove(activeClients.begin(), activeClients.end(), clientSocket), activeClients.end());
    }

    void Cleanup() {
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
        }
        WSACleanup();
    }
};

int main() {
    Server chatServer;
    return chatServer.Start();
}