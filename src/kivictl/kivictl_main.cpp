#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <csignal>
#include <cstring>
#include <kivitree_utils/json.hpp>

using json = nlohmann::json;

void print_banner() {
    std::cout<<"\033[32m"<< R"(                                                      
             ::---::              
         :-:.......:::-.          
       :-:....:::. .  .:--         _  _____     _____ ____ _____ _     
      :-....:=*@@**+-.:::-=       | |/ (_) \   / /_ _/ ___|_   _| |    
     .-:::-+@@+  .-*@#*:-:-.      | ' /| |\ \ / / | | |     | | | |    
     .-..-*%#       -%%=::-:      | . \| | \ V /  | | |___  | | | |___ 
      -:.-*%*-::. =%%@*=--=:      |_|\_\_|  \_/  |___\____| |_| |_____|
      ::.::++%%@@#%@+-::--+       
       -=-:-=----:-=:::-==        
         -+=-::-:::--=+=          
          .=*###*###*+-.          
    )" <<"\033[0m"<<std::endl;
}

std::unordered_map<std::string, std::string> known_binaries = {
    {"kivitree_lb", "./kivitree_lb"},
    {"kivitree_node", "./kivitree_node"}
};

// std::unordered_map<int, std::string> running_procs;
std::unordered_map<std::string, int> running_procs;

void handle_client(int client_socket){
    char buffer[2048] = {0};
    ssize_t valread = read(client_socket, buffer, sizeof(buffer)-1);
    if (valread <=0){
        close(client_socket);
        return;
    }

    std::string input(buffer);
    json req;
    json res;

    try{
        req = json::parse(input);
    }catch(...){
        res["error"] = "Invalid JSON";
        send(client_socket, res.dump().c_str(), res.dump().size(), 0);
        close(client_socket);
        return;
    }

    std::string action = req.value("action", "");
    if (action=="launch") {
        std::string binary = req.value("binary", "");
        if (known_binaries.find(binary) == known_binaries.end()) {
            res["error"] = "Unknown binary";
        } else {
            const std::string& bin_path = known_binaries[binary];
            pid_t pid = fork();
            if (pid == 0) {
                execl(bin_path.c_str(), bin_path.c_str(), nullptr);
                exit(1); // If exec fails
            }
            // running_procs[pid] = binary;
            running_procs[binary] = pid;
            res["status"] = "launched";
            res["pid"] = pid;
            res["binary"] = binary;
        }
    } else if (action == "kill") {
        // int pid = req.value("pid", -1);
        std::string binary = req.value("process", "");
        if (running_procs.find(binary) == running_procs.end()) {
            res["error"] = "PID not found";
        } else {
            int pid = running_procs.at(binary);
            kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0);
            running_procs.erase(binary);
            res["status"] = "terminated";
            res["pid"] = pid;
        }
    } else {
        res["error"] = "Unsupported action";
    }

    std::string response = res.dump();
    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}

int main(){

    int server_fd, new_socket;
    struct sockaddr_in address;
    
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    print_banner();
    std::cout<<"[INFO] kivictl agent listening on port 8080..."<<std::endl;

    std::thread([](){
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::cout<<"[INFO] listening on port 8080..."<<std::endl;
        }
    }).detach();

    while (true){
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        std::thread(handle_client, new_socket).detach();
    }

    return 0;
}