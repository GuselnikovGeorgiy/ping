#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#define PACKET_SIZE     4096
#define PING_TIMEOUT    2
#define DEFAULT_COUNT   4

unsigned short checksum(void *b, int len) {    
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;

    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

void send_ping(int sockfd, struct sockaddr_in *addr) {
    struct icmp *icmp_packet;
    char packet[PACKET_SIZE];
    int packet_size;

    icmp_packet = (struct icmp *)packet;
    memset(icmp_packet, 0, PACKET_SIZE);

    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_id = getpid();
    icmp_packet->icmp_seq = 1;
    icmp_packet->icmp_cksum = checksum(icmp_packet, sizeof(struct icmp));

    packet_size = sizeof(struct icmp);

    if (sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)addr, sizeof(struct sockaddr)) <= 0) {
        perror("sendto");
    }
}

void receive_ping(int sockfd, struct sockaddr_in *addr) {
    char buffer[PACKET_SIZE];
    struct sockaddr_in response_addr;
    socklen_t response_addr_len = sizeof(response_addr);
    struct iphdr *ip_header;
    struct icmp *icmp_packet;
    int bytes_received;

    memset(buffer, 0, PACKET_SIZE);
    bytes_received = recvfrom(sockfd, buffer, PACKET_SIZE, 0, (struct sockaddr *)&response_addr, &response_addr_len);

    if (bytes_received < 0) {
        perror("recvfrom");
    }

    ip_header = (struct iphdr *)buffer;
    icmp_packet = (struct icmp *)(buffer + (ip_header->ihl << 2));
    
    if (icmp_packet->icmp_type == ICMP_ECHOREPLY) {
        printf("Reply from %s: icmp_seq=%u ttl=%d\n",
            inet_ntoa(response_addr.sin_addr), icmp_packet->icmp_seq, ip_header->ttl);
    }
}


int ping_loop(char *ip) {
    struct hostent *host;
    struct sockaddr_in addr;
    int sockfd;
    struct timeval timeout;

    if ((host = gethostbyname(ip)) == NULL) {
        perror("gethostbyname");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr = *((struct in_addr *)host->h_addr);

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket");
        return 1;
    }

    timeout.tv_sec = PING_TIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        return 1;
    }

    printf("PING %s:\n", ip);

    for (int i = 0; i < DEFAULT_COUNT; i++) {
        send_ping(sockfd, &addr);
        receive_ping(sockfd, &addr);
        sleep(1);
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <hostname or IP address>\n", argv[0]);
        return 1;
    }

    ping_loop(argv[1]);

    return 0;
}