#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")

// Function to get the current date and time in a specific format
std::string getCurrentDateTime() {
    std::time_t now = std::time(0);
    struct tm tstruct;
    char buf[80];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

// Function to log a command along with its type and timestamp
void logCommand(const std::string& filePath, char type, const std::string& command) {
    std::ofstream fout(filePath, std::ios::app); 
    if (!fout.is_open()) {
        std::cerr << "Error opening log file" << std::endl;
        return;
    }

    fout << type << "\t" << getCurrentDateTime() << "\t" << "Command [" << command << "] " << std::endl;
}


int main() {
    //Log file
    std::ofstream fout;
    std::string path = "C:\\Users\\super\\OneDrive\\Documents\\Лабороторная по КМ\\2\\server.txt";
    fout.open(path);

    //Initialization of winsocket
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }

    //Create socket
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(1028);

    //Bind socket
    if (bind(server, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Error binding" << std::endl;
        return 1;
    }

    //Looking for connections
    if (listen(server, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening" << std::endl;
        return 1;
    }

    while (true) {
        std::cout << "Server is looking for connections....." << std::endl;
        sockaddr_in client{};
        int clientSize = sizeof(client);
        SOCKET clientSocket = accept(server, (sockaddr*)&client, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting" << std::endl;
            continue;
        }

        std::cout << "Client connected!" << std::endl;
        char buf[4096];
        std::string command;

        do {
            //Receive data from the client
            memset(buf, 0, 4096);
            int bytesReceived = recv(clientSocket, buf, 4096, 0);

            if (bytesReceived <= 0) {
                std::cerr << "Client disconnected or encountered an error." << std::endl;
                break;
            }

            //Log command
            command = std::string(buf, 0, bytesReceived);
            logCommand(path, 'r', command);

            if (command == "who") {
                std::string info = " Kostiantyn Bezruk, K-23, v3";
                send(clientSocket, info.c_str(), info.size() + 1, 0);
                logCommand(path, 's', command);
            }
            else if (command == "dir") {
                std::string dirOutput;
                FILE* pipe = _popen("dir", "r");

                if (!pipe) {
                    std::string error = "Failed to execute 'dir' command.";
                    send(clientSocket, error.c_str(), error.size() + 1, 0);
                    logCommand(path, 's', command);
                    fout << "Error: " << error << std::endl;
                }
                else {
                    char buffer[128];
                    while (!feof(pipe)) {
                        if (fgets(buffer, 128, pipe) != nullptr) {
                            dirOutput += buffer;
                        }
                    }

                    _pclose(pipe);
                    send(clientSocket, dirOutput.c_str(), dirOutput.size() + 1, 0);
                    logCommand(path, 's', command);
                }
            }
            else if (command == "type") {
                std::string filePath = "C:\\Users\\super\\OneDrive\\Documents\\Лабороторная по КМ\\2\\type.txt";
                std::ifstream file(filePath);

                if (file.is_open()) {
                    std::string fileContent((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
                    send(clientSocket, fileContent.c_str(), fileContent.size() + 1, 0);
                    logCommand(path, 's', command);
                    file.close();
                }
                else {
                    std::string error = "File not found or could not be opened.";
                    send(clientSocket, error.c_str(), error.size() + 1, 0);
                    logCommand(path, 's', command);
                    fout << "Error: " << error << std::endl;
                }
            }
            else if (command == "echo") {
                std::string textToEcho = "ECHO mode is enabled.";
                send(clientSocket, textToEcho.c_str(), textToEcho.size() + 1, 0);
                logCommand(path, 's', command);
            }
            else {
                std::string error = "Incorrect input";
                send(clientSocket, error.c_str(), error.size() + 1, 0);
                logCommand(path, 's', command);
                fout << "Error: " << error << std::endl;
            }
        } while (true);

        closesocket(clientSocket);
    }
    //Close sockets
    closesocket(server);
    fout.close();
    WSACleanup();
    return 0;
}