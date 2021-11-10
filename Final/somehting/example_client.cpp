#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
    int sock, portno;
    struct sockaddr_in serv_addr;           
    struct hostent *server;
    char *ip;
    
    // read server ip and port from command line arguments
    if (argc == 3) {
        ip = argv[1];
        portno = atoi(argv[2]);
    } else {
        std::cout << "usage: " << argv[0] << " IP PORT" << std::endl;
    }

    // create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM,  0)) < 0) {
        perror("ERROR opening socket:");
        exit(1);
    }

    // set server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) != 1) {
        perror("invalid ip:");
        exit(1);
    }

    // connect to server
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting:");
        exit(1);
    }

    // set up list of messages
    vector<string> messages;
    messages.push_back("Hi");
    messages.push_back("Hi again");
    messages.push_back("Why am I just saying 'Hi'?");
    messages.push_back("Why are you not answering?");
    messages.push_back("");
    messages.push_back("Bye.");

    // send all messages
    int i = 1;
    for (auto msg:messages) {
        if (send(sock, msg.c_str(), msg.length(), 0) < 0) {
            perror("ERROR sending:");
            exit(1);
        } else {
            std::cout << "msg " << i << ": " << msg << endl;
        }
        i++;
    }
    close(sock);
    return 0;
}
