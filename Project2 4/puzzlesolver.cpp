
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h> 
#include <sys/socket.h> 
#include <sys/ioctl.h>
#include <net/if.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <string>  
#include <stdio.h>   
#include <iostream>
#include <stdint.h>
#include <vector>
#include <fcntl.h>

struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t  placeholder;
    u_int8_t  protocol;
    u_int16_t udp_length;
};
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
        if(response > 0)
        {
            //std::cout << recv_buffer << "\n";
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
    tv.tv_usec = 10000;
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

int set_udp(struct sockaddr_in myaddr)
{
    int udp_sock; 
    //char recv_buffer[4095];
    //int length = sizeof(recv_buffer);

    if((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable to open socket");
        exit(-1);
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 200000;
    setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    return udp_sock;
}

int set_udp_raw() 
{
    struct timeval tv;
    int i = 1;
    int raw_udp_sock;
    if ((raw_udp_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("Unable to create raw socket");
        return -1;
    }
    if (setsockopt (raw_udp_sock, IPPROTO_IP, IP_HDRINCL, &i, sizeof(i)) < 0) {
		perror("Unable to set IP_HDRINCL");
		return -1;
	}
    tv.tv_sec = 0;
    tv.tv_usec = 200000;
    setsockopt(raw_udp_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv));
    return raw_udp_sock;
}

unsigned short csum(unsigned short *ptr,int nbytes) 
{
    /* 
        https://www.binarytides.com/raw-sockets-c-code-linux/, used in the evil bit, 
        creates generic checksum for the evil bit
    */
	long sum;
	unsigned short oddbyte;
	short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer = (short)~sum;
	
	return(answer);
}

std::string evil_bit(const char *ip, const char *local_ip)  
{
    char *udp_payload; 
    char recv_buffer[1025];
    int udp_sock;
    char data[] = "$group_79$";
    int dglen = sizeof(struct ip) + sizeof(struct udphdr) + strlen(data);
    char datagram[dglen];
    struct ip *iph = (struct ip *) datagram;
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip)); // no need to change
    struct sockaddr_in destaddr;
    struct sockaddr_in myaddr; 
    inet_aton(local_ip, &myaddr.sin_addr);// local address 
    myaddr.sin_port = htons(2222);
    myaddr.sin_family = AF_INET;
    int length = sizeof(recv_buffer);

    udp_payload = datagram + sizeof(struct ip) + sizeof(struct udphdr);

    int raw_udp_sock = set_udp_raw();
    int evil_port = 4099; // Needs changing!
    //destaddr.sin_port = htons(2222);
    //destaddr.sin_family = AF_INET;
    inet_aton(ip, &destaddr.sin_addr);

    udp_sock= set_udp(myaddr); // setting normal utp socket for receiving.
    if(bind(udp_sock, (sockaddr *) &myaddr, sizeof(myaddr))<0) 
    {
        perror("Did not bind correctly");
    }

    strcpy(udp_payload, data);

	iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0;
    iph->ip_len = dglen;
    iph->ip_id = htons(55);
    iph->ip_off = 0x8000;
    iph->ip_ttl = 255;
    iph->ip_p = IPPROTO_UDP;
    iph->ip_sum = 0; //csum((unsigned short *) datagram, iph->ip_len);
    iph->ip_src = myaddr.sin_addr;
    iph->ip_dst = destaddr.sin_addr;
    
    udph->uh_ulen =  htons(sizeof (struct udphdr) + strlen(data));
    udph-> uh_sum = 0; //csum((unsigned short *) datagram, sizeof(datagram));
    udph->uh_sport = htons(2222);
    udph-> uh_dport = htons(evil_port);


    bool resp = false; // turns true when response is found
    memset(recv_buffer, 0, sizeof(recv_buffer));
    while (!resp)
    {
        if(sendto(raw_udp_sock, datagram, dglen, 0, (sockaddr *)&destaddr, sizeof(destaddr)) < 0)
        {
            perror("Unable to send offset for evil bit");
        }
        sockaddr_in recvaddr;
        unsigned int recvlen;
        if (recvfrom(udp_sock, recv_buffer, length, 0, NULL, NULL) < 0)
        {
            perror("Received error");
        } else 
        {

            resp = true;
        }
        std::cout << recv_buffer << "\n";
    }
    std::string ret_str = std::string(recv_buffer);
    ret_str.erase(0, ret_str.length()-4);
    std::string temp = "4033,";
    std::cout << temp+ret_str << "\n";
    return temp+ret_str; //The temp concatinates the secret port from before, did unfortunately hardcode it. for now...
}


std::string checksum(const char *ip)
{   
    char *udp_payload; 
    char recv_buffer[1025];
    int udp_sock;
    int check_port = htons(4097); //change to ...
    char data[] = "$group_79$";
    int length = sizeof(recv_buffer);
    struct sockaddr_in destaddr;
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = check_port;
    inet_aton(ip, &destaddr.sin_addr);// local address 
    udp_sock= set_udp(destaddr); // setting normal utp socket for receiving.
    memset(recv_buffer,0,sizeof(recv_buffer));
    bool resp = false; 
    while(!resp)
    {
        if (sendto(udp_sock, data, sizeof(data), 0, (struct sockaddr *) &destaddr, sizeof(destaddr)) < 0)
        {
            perror("Unable to create UDP socket");
        }
        if (recvfrom(udp_sock, recv_buffer, length, 0, NULL, NULL) < 0)
        {
            perror("Unable to receive from udp socket");
        } else {resp = true;}
    }
    std::cout << recv_buffer << "\n";
    

    //extracting ip from msg: 
    std::string msg = std::string(recv_buffer);
    int first = msg.find("being ");
    int last = msg.find("! (the last"); //finds whereeeee it ends
    std::string msg_ip = msg.substr(first + 6, last - first - 6); // finds the ip and assigns it
    std::cout << msg_ip << "\n";

    //extracting checksum from msg: 
    int first_check = msg.find("sum of ");
    int last_check = msg.find(", and with the");
    std::string msg_checks = msg.substr(first_check + 7, last_check - first_check - 7);
    unsigned short msg_checksum = (unsigned short) stoul(msg_checks, nullptr, 16);   
    std::cout << msg_checks << "\n";
    std::cout << msg_checks.length() << "\n";


    //datagram : 
    int dglen = sizeof(struct ip) + sizeof(struct udphdr);
    char datagram[dglen];
    memset(datagram, 0, sizeof(datagram));
    struct ip *iph = (struct ip *) datagram;
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip)); 

	iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0;
    iph->ip_len = htons(dglen);
    iph->ip_id = htons(55);
    iph->ip_off = 0;
    iph->ip_ttl = 255;
    iph->ip_p = IPPROTO_UDP;
    iph->ip_sum = csum((unsigned short *) datagram, dglen);
    iph->ip_src.s_addr = inet_addr(msg_ip.c_str()); // only thing that matters
    iph->ip_dst.s_addr = 0;
    
    udph->uh_ulen =  htons(sizeof (struct udphdr));
    udph-> uh_sum = 0; //csum((unsigned short *) datagram, sizeof(datagram));
    udph->uh_sport = htons(2222);
    udph-> uh_dport = destaddr.sin_port;

    struct pseudo_header psh;
    psh.source_address = iph->ip_src.s_addr;
	psh.dest_address = iph->ip_dst.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.udp_length = udph->uh_ulen;


    int pslen = sizeof(struct pseudo_header) + sizeof(struct udphdr);
    char *pseudogram = (char *)malloc(pslen);
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + strlen(data));


    unsigned short csum_ = ~ntohs(msg_checksum);
    unsigned short init_csum = ~csum((unsigned short *) pseudogram, pslen);
    if (csum_ >= init_csum)
    {
        iph->ip_dst.s_addr = csum_ - init_csum;
    } else {
        iph->ip_dst.s_addr = csum_ - init_csum -1;
    }
    udph->uh_sum = htons(msg_checksum);
    iph->ip_sum = csum((unsigned short *) datagram, dglen);


    int length_d = sizeof(struct ip) + sizeof(struct udphdr);
    memset(recv_buffer,0,sizeof(recv_buffer));
    bool respo = false;
    while(!respo) 
    {
        if (sendto(udp_sock, datagram, length_d, 0 , (struct sockaddr *) &destaddr, sizeof(destaddr )) < 0)
        {
            perror("Sento Failed in checksum");
        }
        if (recvfrom(udp_sock, recv_buffer, length, 0, NULL, NULL) < 0)
        {
            perror("Error in receiving checksum part");
        }
        respo = true;
    }
    std::cout << recv_buffer << "\n";
    
    std::string ret_str = std::string(recv_buffer);
    ret_str = ret_str.substr(55,152);
    return ret_str;
}


void send_oracle(std::string secret_phrase, std::string secret_ports, const char *ip)
{
    struct sockaddr_in server;
    server.sin_port = htons(4042);
    server.sin_family = AF_INET;
    inet_aton(ip, &server.sin_addr);//  address 
    int udp_sock = set_udp(server);
    char recv_buffer[1025];
    memset(recv_buffer,0,sizeof(recv_buffer));
    bool resp = false;
    while (!resp)
    {
        if (sendto(udp_sock, secret_ports.c_str(), secret_ports.length(), 0, (sockaddr *) &server, sizeof(server)) < 0)
        {
            perror("Sendto in oracle not working");
        }
        if (recvfrom(udp_sock, recv_buffer, sizeof(recv_buffer), 0, NULL,NULL) < 0)
        {
            perror("Recvfrom in oracle not working");
        }
        resp = true;
        std::cout << recv_buffer << "\n";
    }
    std::string buffer_string = std::string(recv_buffer);
    while (buffer_string.length() != 0)
    {
        memset(recv_buffer,0,sizeof(recv_buffer));
        int port = stoi(buffer_string.substr(0,4));
        server.sin_port = htons(port);
        if(sendto(udp_sock, secret_phrase.c_str(), secret_phrase.length(), 0, (sockaddr *) &server, sizeof(server)) < 0)
        {
            perror("sento in oracle not working");
        }

        if (recvfrom(udp_sock, recv_buffer, sizeof(recv_buffer), 0, NULL,NULL)<0) 
        {
            perror("not receiving in the oracle part");
        } 
        if (buffer_string.length() == 4)
        {
            buffer_string.erase(0,4);
        } else {buffer_string.erase(0,5);}

    }
    std::cout << recv_buffer<<"\n";
}

int main(int argc, char *argv[])
{
    int from = 4000;
    int to =  4100;
    const char *ip;
    const char *local_ip;
    std::vector<int> ports_found_vector; 
    if (argv[1]) 
    {
        ip = argv[1];
        std::cout << "IP is this one: " << ip << "\n";
    }
    if (argv[2])
    {
        local_ip = argv[2];
    }
    std::cout << "Scanning for ports" << "\n"<< "\n";

    ports_found_vector = scanner_main(from, to, ip);
    for (int i : ports_found_vector) 
    {
        if(sizeof(i) > 0){
            std::cout << i << "\n";
        }
    }
    std::cout << "Done scanning, now for the fun part" << "\n"<< "\n";

    std::cout << "Setting the evil bit..." << "\n"<< "\n";
    std::string secret_ports = evil_bit(ip, local_ip);
    std::cout << "Calculating the checksum..." << "\n"<< "\n";
    std::string secret_phrase = checksum(ip);
    std::cout << "Sending information to oracle..." << "\n"<< "\n";
    send_oracle(secret_phrase, secret_ports, ip);
}