#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 5555
#define BUFFER_SIZE 1024

bool validate(const std::string& response,int n){

    if(response=="invalid input")
        return false;

    std::stringstream ss(response);
    std::vector<int> nums;
    int x;

    while(ss>>x)
        nums.push_back(x);

    if(nums.size()!=10)
        return false;

    for(int v:nums){
        if(v % n !=0)
            return false;
    }

    return true;
}

int main(){

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);

    std::vector<int> tests={1,2,5,10};
    bool ok=true;

    for(int n:tests){

        SOCKET sock=socket(AF_INET,SOCK_STREAM,0);

        sockaddr_in serv_addr{};
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_port=htons(PORT);

        inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);

        connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr));

        std::string msg=std::to_string(n);

        send(sock,msg.c_str(),msg.size(),0);

        char buffer[BUFFER_SIZE]={0};

        int valread=recv(sock,buffer,BUFFER_SIZE,0);

        std::string resp(buffer,valread);

        if(!validate(resp,n)){
            ok=false;
            std::cout<<"Test failed for n="<<n<<"\n";
        }else{
            std::cout<<"Test ok for n="<<n<<"\n";
        }

        closesocket(sock);
    }

    WSACleanup();

    return ok?0:1;
}
