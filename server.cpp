#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>
#include <sstream>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 5555
#define BUFFER_SIZE 1024
#define WORKERS 4

struct Task {
    SOCKET client_socket;
    std::string data;
};

std::queue<Task> tasks;
std::mutex mtx;
std::condition_variable cv;

bool parseInt(const std::string &s, int &result) {
    try {
        size_t pos;
        int val = std::stoi(s, &pos);
        if (pos != s.size() || val <= 0) return false;
        result = val;
        return true;
    } catch (...) {
        return false;
    }
}

std::string generateResponse(int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1,100);

    std::string res;
    for(int i=0;i<10;i++){
        res += std::to_string(dist(gen)*n);
        if(i!=9) res+=" ";
    }
    return res;
}

void worker(){
    while(true){
        Task task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock,[]{return !tasks.empty();});
            task=tasks.front();
            tasks.pop();
        }

        int n;
        std::string response;

        if(!parseInt(task.data,n))
            response="invalid input";
        else
            response=generateResponse(n);

        send(task.client_socket,response.c_str(),response.size(),0);
    }
}

int main(){

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);

    SOCKET server_fd = socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(PORT);

    bind(server_fd,(sockaddr*)&address,sizeof(address));
    listen(server_fd,10);

    for(int i=0;i<WORKERS;i++)
        std::thread(worker).detach();

    fd_set master_set,read_fds;
    FD_ZERO(&master_set);
    FD_SET(server_fd,&master_set);

    while(true){
        read_fds=master_set;

        select(0,&read_fds,nullptr,nullptr,nullptr);

        for(u_int i=0;i<read_fds.fd_count;i++){

            SOCKET s=read_fds.fd_array[i];

            if(s==server_fd){
                SOCKET new_socket=accept(server_fd,nullptr,nullptr);
                FD_SET(new_socket,&master_set);
            }
            else{
                char buffer[BUFFER_SIZE]={0};
                int valread=recv(s,buffer,BUFFER_SIZE,0);

                if(valread<=0){
                    closesocket(s);
                    FD_CLR(s,&master_set);
                }else{
                    Task t;
                    t.client_socket=s;
                    t.data=std::string(buffer,valread);

                    {
                        std::lock_guard<std::mutex> lock(mtx);
                        tasks.push(t);
                    }
                    cv.notify_one();
                }
            }
        }
    }

    WSACleanup();
}

