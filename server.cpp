// server.cpp
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 12345;

// Функція для обробки одного клієнта
void handle_client(SOCKET client_socket) {
    char buffer[1024];

    while (true) {
        int valread = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (valread <= 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        buffer[valread] = '\0';
        std::string input(buffer);
        std::string response;

        try {
            int n = std::stoi(input);  // спробуємо перетворити рядок у число
            for (int i = 0; i < 10; ++i) {
                int r = (rand() % 100 + 1) * n;
                response += std::to_string(r) + " ";
            }
        } catch (...) {
            response = "invalid input";
        }

        send(client_socket, response.c_str(), response.size(), 0);
    }

    closesocket(client_socket);
}

int main() {
    srand((unsigned int)time(0));

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started on port " << PORT << "\n";

    std::vector<std::thread> threads;

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::cout << "New client connected\n";
        // Створюємо потік для кожного клієнта
        threads.emplace_back(handle_client, client_socket);
        threads.back().detach(); // відпускаємо потік
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

