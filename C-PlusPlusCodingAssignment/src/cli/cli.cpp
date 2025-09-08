#include "comms.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <filesystem>

std::string host = "127.0.0.1";
int port = 5555;
bool persistent_mode = true;

void load_config() {
    std::string config_path = std::filesystem::current_path().string() + "/config";
    std::ifstream f(config_path);
    if (!f) {
        return;
    }
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string key, val;
        if (std::getline(iss, key, '=') && std::getline(iss, val)) {
            if (key == "host") { 
                host = val;
            }
            else if (key == "port") {
                port = std::stoi(val);
            }
            else if (key == "persistent") 
                persistent_mode = (val == "true");
        }
    }
}

void menu(int sock_fd) {
    while (true) {
        std::cout << "\n1. Insert\n2. Delete\n3. Print all\n4. Delete all\n5. Find\n6. Exit\nChoice: ";
        int choice; 
        // Validate input
        if (!(std::cin >> choice)) {
            std::cout << "Invalid input! Please enter a number between 1 and 6.\n";
            std::cin.clear(); // clear failbit
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard invalid input
            continue;
        }

        if (choice < 1 || choice > 6) {
            std::cout << "Invalid choice! Please enter a number between 1 and 6.\n";
            continue;
        }

        // Clear newline left in buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::ostringstream msg;
        if (choice == 1) {
            std::cout << "Enter number: ";
            int n; std::cin >> n;   
            msg << "Insert " << n; 
        }
        else if (choice == 2) { 
            std::cout << "Enter number: "; 
            int n; std::cin >> n; 
            msg << "Delete " << n; 
        }
        else if (choice == 3) 
            msg << "PrintAll";
        else if (choice == 4) 
            msg << "DeleteAll";
        else if (choice == 5) { 
            std::cout << "Enter number: ";
            int n; 
            std::cin >> n; 
            msg << "Find " << n; 
        }
        else if (choice == 6) { 
            msg << "Exit";
            send_message(sock_fd, msg.str()); 
            std::cout << recv_message(sock_fd) << "\n"; 
            break; 
        }
        else { 
            std::cout << "Invalid choice\n"; 
            continue; 
        }
        bool ret = send_message(sock_fd, msg.str());
        if(ret == false){
            std::cout << "Connection closed";
        } else {
            std::string resp = recv_message(sock_fd);
            std::cout << "Response: " << resp << "\n";
        }

        if (!persistent_mode) break;
    }
}

int main(int argc, char *argv[]) {
    load_config();
    if (argc > 1) 
        host = argv[1];
    if (argc > 2) 
        port = std::stoi(argv[2]);
    if (argc > 3) 
        persistent_mode = (std::string(argv[3]) == "true");

    std::cout << "Connecting on Port : " << port << " host : " << host << "\n";

    if (persistent_mode) {
        int sock_fd = connect_to_server(host, port);
        if (sock_fd < 0) return 1;
        menu(sock_fd);
        close(sock_fd);
    } else {
        while (true) {
            int sock_fd = connect_to_server(host, port);
            if (sock_fd < 0) return 1;
            menu(sock_fd);
            close(sock_fd);
            break;
        }
    }
}

