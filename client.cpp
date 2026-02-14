#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cout << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);  // порт сервера
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Connection failed\n";
        closesocket(sock);
        return 1;
    }

    std::string input;
    while (true) {
        std::cout << "Enter n (or 'exit' to quit): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        // Надсилаємо запит серверу
        send(sock, input.c_str(), input.size(), 0);

        // Отримуємо відповідь
        char buffer[1024] = {0};
        int valread = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            std::cout << "Server response: " << buffer << "\n";
        } else {
            std::cout << "No response or connection closed\n";
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
