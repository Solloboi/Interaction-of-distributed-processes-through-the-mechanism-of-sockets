#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>

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

// Function to send and receive commands from the server
void SendAndReceiveCommands(SOCKET client, const std::string& input, std::ofstream& fout) {
    std::istringstream ss(input);
    std::string singleCommand;
    while (std::getline(ss, singleCommand, ' ')) {
        if (singleCommand.empty()) {
            continue;
        }

        // Send the command to the server
        send(client, singleCommand.c_str(), singleCommand.size() + 1, 0);
        fout << "r\t" << getCurrentDateTime() << "\tCommand [" << singleCommand.c_str() << "] was sent" << std::endl;

        // Receive the response from the server
        char buf[4096];
        int bytes = recv(client, buf, sizeof(buf), 0);

        if (bytes > 0) {
            std::cout << "Response from the server: " << std::string(buf, bytes) << std::endl;
            fout << "s\t" << getCurrentDateTime() << "\tResponse to command [" << singleCommand << "] was received" << std::endl;
        }
        else {
            std::cerr << "Error receiving response from the server" << std::endl;
            break;
        }
    }
}

int main() {
    std::ofstream fout;
    std::string path = "C:\\Users\\super\\OneDrive\\Documents\\Лабороторная по КМ\\2\\client.txt";
    fout.open(path);

    //Initialization of winsocket
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        std::cerr << "Failed to initialize socket" << std::endl;
        return 1;
    }

    //Create socket
    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(1028);

    // Connect to the server
    if (connect(client, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to the server" << std::endl;
        closesocket(client);
        WSACleanup();
        return 1;
    }

    // Start of communiaction with user
    std::string inputChoice;
    

    do {
        std::cout << "Do you want to start client? (YES/EXIT): ";
        std::cin >> inputChoice;
        if (inputChoice == "YES") {
            std::cin.ignore();
            while (true) {
                std::string input;
                std::cout << "Enter commands ('EXIT / who / dir / type / echo'): ";
                std::getline(std::cin, input);

                if (input == "EXIT") {
                    send(client, input.c_str(), input.size() + 1, 0);
                    closesocket(client);
                    break;
                }

                SendAndReceiveCommands(client, input, fout);
            }
        }
        else if (inputChoice == "EXIT") {
            closesocket(client);
            break;
        }
        else {
            std::cerr << "Invalid choice." << std::endl;
            closesocket(client);
            continue;
        }
    }while (true);


    //Close sockets
    closesocket(client);
    WSACleanup();
    fout.close();
    return 0;
}