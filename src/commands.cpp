#include "commands.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>

#define STORAGE_DIR "storage/"

std::string processCommand(const std::string& command, int clientSocket) {
    std::istringstream iss(command);
    std::string cmd, filename;
    iss >> cmd >> filename;

    if (cmd == "LIST") return listFiles();
    if (cmd == "GET") return getFile(filename, clientSocket);
    if (cmd == "PUT") return putFile(filename, clientSocket);
    if (cmd == "DELETE") return deleteFile(filename);

    return "ERROR: Unknown command\n";
}

std::string listFiles() {
    std::string fileList;
    for (const auto& entry : std::filesystem::directory_iterator(STORAGE_DIR)) {
        fileList += entry.path().filename().string() + "\n";
    }
    return fileList.empty() ? "No files found\n" : fileList;
}

std::string getFile(const std::string& filename, int clientSocket) {
    std::ifstream file(STORAGE_DIR + filename, std::ios::binary);
    if (!file.is_open()) return "ERROR: File not found\n";

    char buffer[1024];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        send(clientSocket, buffer, file.gcount(), 0);
    }

    return "FILE_SENT\n";
}

std::string putFile(const std::string& filename, int clientSocket) {
    std::ofstream file(STORAGE_DIR + filename, std::ios::binary);
    if (!file.is_open()) return "ERROR: Cannot create file\n";

    char buffer[1024];
    int bytesRead;
    while ((bytesRead = read(clientSocket, buffer, sizeof(buffer))) > 0) {
        file.write(buffer, bytesRead);
    }

    return "UPLOAD_SUCCESS\n";
}

std::string deleteFile(const std::string& filename) {
    return std::filesystem::remove(STORAGE_DIR + filename) ? "DELETE_SUCCESS\n" : "ERROR: File not found\n";
}
