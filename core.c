#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <signal.h>

#define DEFAULT_PACKET_SIZE 64
#define PING_TIMEOUT    2
#define DEFAULT_COUNT   4

unsigned short checksum(void *b, int len) {   
    /* 
        Проверяем контрольную сумму ICMP пакета
    */ 
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

int send_request(int sockfd, struct sockaddr_in *addr, int seq_num) {
    /*
        Создаем буфер для ICMP пакета в виде структуры,
        заполняем буфер, вычисляем контрольную сумму,
        отправляем пакет.
    */
    char packet[DEFAULT_PACKET_SIZE];
    memset(packet, 0, DEFAULT_PACKET_SIZE);

    struct icmp *icmp_packet = (struct icmp *)packet;
    // Структура заголовка для ICMP
    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_id = getpid(); // pid процесса
    icmp_packet->icmp_seq = seq_num;
    icmp_packet->icmp_cksum = 0;

    gettimeofday((struct timeval *)icmp_packet->icmp_data, NULL); // Временные метки

    // Считаем контрольную сумму пакета
    icmp_packet->icmp_cksum = in_cksum((unsigned short *)icmp_packet, DEFAULT_PACKET_SIZE);

    // Отправляем запрос
    if (sendto(sockfd, packet, DEFAULT_PACKET_SIZE, 0, (struct sockaddr *)addr, sizeof(*addr)) == -1) {
        perror("Ошибка отправки запроса к адресу: send_request");
        return 1;
    }

    return 0;
}

int receive_response(int sockfd, struct sockaddr_in *addr, int seq_num) {
    /*
        Создаем буфер для ICMP пакета, получаем пакет, обрабатываем
        ответ, выводим информацию о пакете.
    */
   
    // Буфер
    char buffer[DEFAULT_PACKET_SIZE];
    memset(buffer, 0, DEFAULT_PACKET_SIZE);

    // адрес и размер адреса
    struct sockaddr_in response_addr;
    socklen_t response_addr_len = sizeof(response_addr);

    // Получение ответа от хоста
    int bytes_received = recvfrom(sockfd, buffer, DEFAULT_PACKET_SIZE, 0, (struct sockaddr *)&response_addr, &response_addr_len);
    if (bytes_received < 0) {
        perror("Ошибка получения запроса от адреса: receive_response");
        return 1;
    }
    
    // Проверка, что ответ пришел не от целевого хоста (крайне маловероятно)
    if (response_addr.sin_addr.s_addr != addr->sin_addr.s_addr) {
        printf("Получено сообщение не от целевого хоста\n");
        return -1;
    }

    // Получаем заголовки для IP/ICMP
    struct iphdr *ip_header = (struct iphdr *)buffer;
    struct icmp *icmp_packet = (struct icmp *)(buffer + (ip_header->ihl << 2));
    
    // Packet = icmp_echo_reply и ожидаемый номер запроса
    if (icmp_packet->icmp_type == ICMP_ECHOREPLY && icmp_packet->icmp_seq == seq_num) {
        
        struct timeval *sent_time = (struct timeval *)icmp_packet->icmp_data;
        struct timeval received_time;
        gettimeofday(&received_time, NULL);
        // Вычисление round-trip time (дельта t2-t1)
        double rtt = (received_time.tv_sec - sent_time->tv_sec) * 1000.0 + (received_time.tv_usec - sent_time->tv_usec) / 1000.0;
        
        // Вывод информации о пакете
        printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
               bytes_received, inet_ntoa(response_addr.sin_addr), icmp_packet->icmp_seq, ip_header->ttl, rtt);
    } else {
        printf("Получен непредвиденный ICMP ответ\n");
        return -2;
    }

    return 0;
    
}


int ping_loop(char *ip) {
    /*
        Главный цикл пинга, создаем сокет, устанавливаем подключение,
        в этом цикле происходит отправка запросов и прием ответов от хоста.
        Вывод статистики о подключении.
    */
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

    int seq_num = 0;

    for (int i = 0; i < DEFAULT_COUNT; i++) {
        send_request(sockfd, &addr, seq_num);
        receive_response(sockfd, &addr);
        sleep(1);
        ++seq_num;
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {
    /*
        Запуск главного цикла.
        *** TODO: вынести проверку в отдельную функцию "CheckArgs"
    */
    if (argc != 2) {
        printf("Usage: %s <hostname or IP address>\n", argv[0]);
        return 1;
    }

    ping_loop(argv[1]);

    return 0;
}