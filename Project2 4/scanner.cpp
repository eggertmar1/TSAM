#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <vector>
#include <string>
//#include "scanner.hpp"
// sd

int scan(int socket,  char* send_buffer, sockaddr_in destaddr, char* recv_buffer,  int length)
{
    memset(recv_buffer, 0, strlen(recv_buffer));
    //unsigned int len;
    //struct sockaddr_in receive_addr;
    int ports_found = 0;
    for(int i = 0; i <=5; i++)
    {
        if (sendto(socket, send_buffer, strlen(send_buffer), 0x0, (const struct sockaddr *) &destaddr, sizeof(destaddr)) < 0)
            {
                perror("Unable to send packets");
                return 0;
            } 
        int response= recvfrom(socket, recv_buffer, length, 0,  NULL, NULL);
        std::cout << recv_buffer << "\n";
        if(response > 0)
        {
            return response;
        }
    }
    return 0;
}
std::vector<int> scanner_main(int from,int to, const char *ip) 
{
    int udp_sock; 
    char send_buffer[1025];
    char recv_buffer[1025];
    int length;
    struct sockaddr_in destaddr; 
    struct sockaddr_in receive_addr;

    std::vector<int> ports_found_vector; 
    
    length = sizeof(recv_buffer);
    strcpy(send_buffer, "$group_79$\0");


    if((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable to open socket");
        exit(-1);
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 20000; 
    setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    destaddr.sin_family = AF_INET;

    inet_aton(ip, &destaddr.sin_addr);

    for (int port = from; port <= to; port++)
    {
        destaddr.sin_port = htons(port);
        int response = scan(udp_sock, send_buffer, destaddr, recv_buffer, length);
        if (response > 0) 
        {
            ports_found_vector.push_back(port);
        }
    }
    // for (int i : ports_found_vector) 
    // {
    //     std::cout << i << "\n";
    // }
    //std::cout << recv_buffer << "\n";
    return ports_found_vector;
}



int main(int argc, char *argv[]) 
{
    int from = atoi(argv[2]);
    int to =  atoi(argv[3]);
    const char *ip;
    std::vector<int> ports_found_vector; 
    if (argv[1]) 
    {
        ip = argv[1];
        std::cout << "IP is this one: " << ip << "\n";
    }
    ports_found_vector = scanner_main(from, to, ip);
}
