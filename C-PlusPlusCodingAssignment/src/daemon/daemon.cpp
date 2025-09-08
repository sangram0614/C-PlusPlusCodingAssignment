#include "comms.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <set>
#include <chrono>
#include <fstream>
#include <sstream>
#include <map>
#include <atomic>
#include <unistd.h>

std::set<int> numbers;
std::map<int, time_t> timestamps;
std::mutex data_mutex;

struct Task {
    int client_fd;
};

std::queue<Task> task_queue;
std::mutex queue_mutex;
std::condition_variable cv;
std::atomic<bool> stop_flag(false);

bool persistent_mode = true;  // default
int thread_pool_size = 4;
int port = 5555;

void log_message(const std::string &msg) {
    std::ofstream log("daemon.log", std::ios::app);
    log << "[" << time(nullptr) << "] " << msg << std::endl;
}

class SocketGuard {
    int fd;
public:
    SocketGuard(int fd) : fd(fd) {}
    ~SocketGuard() { 
        if (fd >= 0) 
        close(fd);
        log_message("Cloasing the connection : " + std::to_string(fd));
    }
};


std::string process_command(const std::string &cmd) {
    std::istringstream iss(cmd);
    std::string op;
    iss >> op;
    std::ostringstream oss;

    std::lock_guard<std::mutex> lock(data_mutex);

    if (op == "Insert") {
        int num;
        iss >> num;
        if (num <= 0) return "Error: Only positive integers allowed";
        if (numbers.count(num)) return "Error: Duplicate not allowed";
        numbers.insert(num);
        timestamps[num] = time(nullptr);
        oss << "Inserted " << num << " at " << timestamps[num];
    } else if (op == "Delete") {
        int num; iss >> num;
        if (!numbers.count(num)) return "Error: Not found";
        numbers.erase(num);
        timestamps.erase(num);
        oss << "Deleted " << num;
    } else if (op == "PrintAll") {
        oss << "Stored numbers:\n";
        for (int n : numbers) {
            oss << n << " (" << timestamps[n] << ")\n";
        }
    } else if (op == "DeleteAll") {
        numbers.clear();
        timestamps.clear();
        oss << "All numbers deleted";
    } else if (op == "Find") {
        int num; iss >> num;
        if (numbers.count(num)) oss << "Found " << num;
        else oss << num << " not found";
    } else if (op == "Exit") {
        oss << "Goodbye!";
    } else {
        oss << "Error: Unknown command";
    }

    log_message(oss.str());
    return oss.str();
}

void handle_client(int client_fd) {
    SocketGuard guard(client_fd);

    if (persistent_mode) {
        while (true) {
            std::string msg = recv_message(client_fd);
            if (msg.empty()) break;
            std::string response = process_command(msg);
            send_message(client_fd, response);
            if (msg == "Exit") break;
        }
    } else {
        std::string msg = recv_message(client_fd);
        if (!msg.empty()) {
            std::string response = process_command(msg);
            send_message(client_fd, response);
        }
    }
}

void worker_thread() {
    while (!stop_flag) {
        Task task{-1};
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [] { return !task_queue.empty() || stop_flag; });
            if (stop_flag) return;
            task = task_queue.front();
            task_queue.pop();
        }
        handle_client(task.client_fd);
    }
}

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
            if (key == "threads") 
                thread_pool_size = std::stoi(val);
            else if (key == "port") 
                port = std::stoi(val);
            else if (key == "persistent") 
                persistent_mode = (val == "true");
        }
    }
}

int main(int argc, char *argv[]) {
    load_config();
    if (argc > 1) 
        thread_pool_size = std::stoi(argv[1]);
    if (argc > 2) 
        port = std::stoi(argv[2]);
    if (argc > 3) 
        persistent_mode = (std::string(argv[3]) == "true");

    //std::cout << "Listning on Port : " << port << " thread pool size : " << thread_pool_size << "\n";
    int server_fd = create_server_socket(port);
    if (server_fd < 0) 
        return 1;

    log_message("Daemon started on port " + std::to_string(port));

    std::vector<std::thread> workers;
    for (int i = 0; i < thread_pool_size; i++)
        workers.emplace_back(worker_thread);

    while (true) {
        int client_fd = accept_client(server_fd);
        if (client_fd < 0) continue;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            task_queue.push({client_fd});
        }
        log_message("CLI client connected : " + std::to_string(client_fd));
        cv.notify_one();
    }

    stop_flag = true;
    cv.notify_all();
    for (auto &w : workers) w.join();
    close(server_fd);
}

