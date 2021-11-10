// Simple configuration of socket. 
// Sends hardcoded msg to the server 
// Reads back response. 

// Connecting: ./client <ip> <port number>

// compile: g++ -o client client.cpp
// Author: 
// Eggert Mar Eggertsson (eggerte19@ru.is)
// int socket(int domain, int type, int protocol);
// for this assignment socket(PF_INET, SOCK_STREAM,0)
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
using namespace std; 


int main(int argc, char *argv[]){
	char *msg = "SYS ls -sal"; 
	char buffer[4096];
	int valread;
	struct hostent *host;
	printf("%d\n", argc);  	        // how many arguments
	printf("IP: %s\n", argv[1]);	// ip given
	printf("PORT no: %s\n", argv[2]); // port number 
	string ip;
	int sockfd;
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(5000);

	if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <=0)
	{
		printf("\nInvalid address\n");
		return -1;
	}
	if (connect(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr))<0)
	{
		printf("\nConnection failed\n");
		return -1;
	}
	bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	char *hostname = argv[1];
	host = gethostbyname(hostname);
	if (host == NULL) {
		fprintf(stderr,"ERROR, no such host\n"); exit(0);
	}
	int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

	//send(sockfd , msg , strlen(msg) , 0 );	//sending message to server
    //printf("Message sent\n"); 		
	char input[32];
	while (1) {
		std::cin.getline(input, sizeof(input));
		send(sockfd, input, sizeof(input),0);
		recv(sockfd, buffer, sizeof(buffer), 0); //getting the response
		printf("%s\n",buffer);			//printing response
        memset(buffer, 0, sizeof(buffer));
//        memset(input, 0, sizeof(input));
	}	
}
