#include "server.h"
#include "commands.h"
#include "thread_pool.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

#define PORT 2121  // FTP servers usually run on port 21, but we'll use 2121

std::unordered_map<std::string, std::string> users;  // Stores username-password pairs

// 🔹 Load users from `users.txt`
void loadUsers() {
    std::ifstream file("users.txt");
    std::string line, user, pass;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (iss >> user >> pass) {
            users[user] = pass;
            std::cout << "Loaded user: " << user << std::endl;  // Debugging line
        }
    }
}

// 🔹 Authenticate user
bool authenticate(int clientSocket) {
    char buffer[1024] = {0};

    // Read username
    send(clientSocket, "Username: ", 10, 0);
    read(clientSocket, buffer, 1024);
    std::string username(buffer);
    username.erase(username.find_last_not_of(" \n\r\t") + 1);

    // Check if user exists
    if (users.find(username) == users.end()) {
        std::string register_prompt = "User not found. Register? (yes/no): ";
        send(clientSocket, register_prompt.c_str(), register_prompt.length(), 0);
        memset(buffer, 0, sizeof(buffer));
        read(clientSocket, buffer, 1024);
        std::string response(buffer);
        response.erase(response.find_last_not_of(" \n\r\t") + 1);

        if (response == "yes") {
            std::string password_prompt = "Enter new password: ";
            send(clientSocket, password_prompt.c_str(), password_prompt.length(), 0);
            memset(buffer, 0, sizeof(buffer));
            read(clientSocket, buffer, 1024);
            std::string newPassword(buffer);
            newPassword.erase(newPassword.find_last_not_of(" \n\r\t") + 1);

            // Save new user
            users[username] = newPassword;
            std::ofstream file("users.txt", std::ios::app);
            file << username << " " << newPassword << "\n";
            file.close();

            std::string success_msg = "Registration successful! Please reconnect.\n";
            send(clientSocket, success_msg.c_str(), success_msg.length(), 0);
            memset(buffer, 0, sizeof(buffer));
            read(clientSocket, buffer, 1024);  // Wait for client to acknowledge
            close(clientSocket);
            return false;
        } else {
            send(clientSocket, "AUTH_FAILED\n", 12, 0);
            close(clientSocket);
            return false;
        }
    }

    // Read password
    send(clientSocket, "Password: ", 10, 0);
    read(clientSocket, buffer, 1024);
    std::string password(buffer);
    password.erase(password.find_last_not_of(" \n\r\t") + 1);

    if (users[username] == password) {
        send(clientSocket, "AUTH_SUCCESS\n", 13, 0);
        return true;
    }

    send(clientSocket, "AUTH_FAILED\n", 12, 0);
    close(clientSocket);
    return false;
}


// 🔹 Handle each client request
void handleClient(int clientSocket) {
    if (!authenticate(clientSocket)) return;

    char buffer[1024] = {0};

    while (true) {
        int bytesRead = read(clientSocket, buffer, 1024);
        if (bytesRead <= 0) break;

        std::string command(buffer, bytesRead);
        command.erase(command.find_last_not_of(" \n\r\t") + 1);

        if (command == "QUIT") break;

        std::string response = processCommand(command, clientSocket);
        send(clientSocket, response.c_str(), response.length(), 0);
    }

    close(clientSocket);
}

// 🔹 Start FTP Server
void startFTPServer() {
    loadUsers();  // Load authentication data

    int serverFd, clientSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(serverFd, (struct sockaddr*)&address, sizeof(address));
    listen(serverFd, 5);
    std::cout << "FTP Server Listening on Port " << PORT << std::endl;

    ThreadPool threadPool(4);  // Multi-threaded handling

    while ((clientSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0) {
        threadPool.enqueue([clientSocket] { handleClient(clientSocket); });
    }
}
