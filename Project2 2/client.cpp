#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>

//130.208.242.120

int main(int argc, char *argv[]) 
{
    int udp_sock; 
    int from = atoi(argv[2]);
    int to =  atoi(argv[3]);
    char send_buffer[1400];
    char recv_buffer[1400];
    int length;
    struct sockaddr_in destaddr; 
    struct sockaddr_in receive_addr;
    unsigned int len;
    const char * ip;
    std::list<int> listofPorts;

    strcpy(send_buffer, "Hi Port!");
    length = strlen(send_buffer) + 1;


    if((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable to open socket");
        return(0);
    }

    if (argv[1]) 
    {
        ip = argv[1];
        std::cout << "IP is this one: " << ip << "\n";
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 20000;
    setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    destaddr.sin_family = AF_INET;

    inet_aton(ip, &destaddr.sin_addr);

    for(int port = from; port<= to; port++) 
    {
        destaddr.sin_port = htons(port);
        for (int i = 0; i<= 5; i++) // Iterating five times because UDP is not reliable enough.
        {
            if (sendto(udp_sock, send_buffer, length, 0, (const struct sockaddr *) &destaddr, sizeof(destaddr)) < 0)
            {
                perror("Unable to send packets");
            } 
            if (recvfrom(udp_sock, recv_buffer, length, 0, (struct sockaddr *) &receive_addr, &len) > 0)
            {
                std::cout << "Port: " << port << "\n";
                listofPorts.push_back(port);
                std::cout << recv_buffer<< "\n";
            } 
        }
    }   
}