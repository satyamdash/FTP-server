#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

std::string processCommand(const std::string& command, int clientSocket);
std::string listFiles();
std::string getFile(const std::string& filename, int clientSocket);
std::string putFile(const std::string& filename, int clientSocket);
std::string deleteFile(const std::string& filename);

#endif
