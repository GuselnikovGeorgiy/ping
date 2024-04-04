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
#include <regex.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>


#define DEFAULT_PACKET_SIZE 64  //bytes
#define PING_TIMEOUT    2     //seconds
#define DEFAULT_COUNT   4     //numbers of requests
#define DEFAULT_SLEEP_TIME 1  //seconds

int interrupted = 0;

int count = DEFAULT_COUNT;
int loop = 0;
char *path = "";  // default path to log
char *ipv4 = "";



void sigint_handler(int sigint) {
    interrupted = 1;
}

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
    icmp_packet->icmp_cksum = checksum((unsigned short *)icmp_packet, DEFAULT_PACKET_SIZE);

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
        printf("Request timed out...\n");
        return 0;
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
        double rtt = (received_time.tv_sec - sent_time->tv_sec) * 1000.0 + 
                    (received_time.tv_usec - sent_time->tv_usec) / 1000.0;
        
        // Вывод информации о пакете
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
               bytes_received, inet_ntoa(response_addr.sin_addr), icmp_packet->icmp_seq, ip_header->ttl, rtt);
    } else {
        printf("Получен непредвиденный ICMP ответ\n");
        return -2;
    }

    return 0;
    
}

int print_statisctics(int packets_sent, int packets_received, double total_time) {
    /*
        Вывод статистики
    */
    printf("\n--- Ping statistics ---\n");
    printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.2fms\n",
           packets_sent, packets_received, 
           ((double)(packets_sent - packets_received) / packets_sent) * 100, total_time);
    
    return 0;
}


int requests_loop(const char *ip, int count, int loop) {
    /*
        Главный цикл пинга, создаем сокет, устанавливаем подключение,
        в этом цикле происходит отправка запросов и прием ответов от хоста.
        Вывод статистики о подключении.
    */

    signal(SIGINT, sigint_handler);

    int sockfd; // Дескриптор сокета

    int seq_num = 0;
    int packets_sent = 0;
    int packets_received = 0;

    struct sockaddr_in addr;
    struct timeval start_time, end_time;
    double total_time = 0;

    // Создаем сокет для общения с хостом
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket");
        return 1;
    }

    // Структура адреса
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);

    gettimeofday(&start_time, NULL);

    // Устанавливаем таймаут для пинга
    struct timeval timeout;
    timeout.tv_sec = PING_TIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        return 1;
    }

    printf("Pinging %s, with %d bytes of data:\n", ip, DEFAULT_PACKET_SIZE);

    while (!interrupted) {
        
        if (packets_sent >= count && !loop) {
            break;
        }

        if (send_request(sockfd, &addr, seq_num) != 0) {
            break;
        }
        if (receive_response(sockfd, &addr, seq_num) != 0) {
            break;
        }

        ++seq_num;
        ++packets_sent;
        ++packets_received;

        sleep(DEFAULT_SLEEP_TIME);
    }

    gettimeofday(&end_time, NULL);

    total_time = (double)(end_time.tv_sec - start_time.tv_sec) * 1000 +
                 (double)(end_time.tv_usec - start_time.tv_usec) / 1000;

    // Вывод статистики
    print_statisctics(packets_sent, packets_received, total_time);

    close(sockfd);
    return 0;
}


int validate_ip(const char *ip) {
    /*
        Проверяем ip на валидность
    */
    regex_t regex;
    int result;

    char *pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";

    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result) {
        fprintf(stderr, "Невозможно скомпилировать регулярное выражение\n");
        return -1;
    }

    result = regexec(&regex, ip, 0, NULL, 0);
    regfree(&regex);

    if (!result) {
        return 0; // соответствует IPv4
    } else if (result == REG_NOMATCH) {
        printf("Задан неверный IP\n");
        return 1;
    } else {
        fprintf(stderr, "Ошибка при работе с регулярным выражением\n");
        return -1;
    }
}

int is_int(const char *num) {
    char *endptr;
    strtol(num, &endptr, 10);

    if ((errno == ERANGE && (strtol(num, NULL, 10) == LONG_MAX || strtol(num, NULL, 10) == LONG_MIN)) || (errno != 0 && strtol(num, NULL, 10)== 0))
        return 1;
    if (*endptr == '\0')
        return 0;
    return 1;
}


int check_args(int argc, char *argv[]) {
    /*
        Проверяем входные аргументы при запуске программы
    */

    ipv4 = argv[1];

    if (argc < 2 || argc > 4) {
        printf("Usage: %s <IPv4> [log_dir] [num_count:int |-t]\n", argv[0]);
        return -1;
    }
    
    int flag_count = 0;
    int flag_loop = 0;
    int flag_log = 0;

    if (validate_ip(ipv4) == 0) {
        for (int i=2; i < argc; ++i) {
            if (strcmp(argv[i], "-t") == 0) {
                if (!flag_loop & !flag_count) {
                    loop = 1;
                    flag_loop = 1;
                    continue;
                }
                else {
                    printf("Недопустимое значение: %s.\n", argv[i]);
                    return 1;
                }
            } else if (is_int(argv[i]) == 0) {
                if (!flag_count & !flag_loop) {
                    count = atoi(argv[i]);
                    flag_count = 1;
                    continue;
                } else {
                    printf("Недопустимое значение: %s.\n", argv[i]);
                    return 1;
                }
            } else if (access(argv[i], W_OK) != -1 && access(argv[i], F_OK) != -1) {
                if (!flag_log) {
                    path = argv[i];
                    printf("Write log to: %s\n", argv[i]);
                    flag_log++;
                    continue;
                }
            }

        }
        if (argc > 3 && !flag_log && (flag_count || flag_loop)) {
            printf("Файл недоступен или не существует.\n");
            return 1;
        }

        return 0;
    }

    return 1;
}

void finish() {
    exit(0);
}

int check_log() {
    return 0;
}

int diag_check_log() {
    return 0;
}

int create_log() {
    return 0;
}

int diag_create_log() {
    return 0;
}

int diag_check_args() {
    return 0;
}


int diag_requests_loop() {
    return 0;
}


int main(int argc, char *argv[]) {

    /* Проверка аргументов */
    switch(check_args(argc, argv)) {
        /* Аргументы верные */
        case 0:
            /* Проверка наличия лога */
            switch(check_log()) {
                /* Лог есть, ничего делать не надо */
                case 0:
                    /* Цикл запросов */
                    switch (requests_loop(ipv4, count, loop)) {
                        /* Конец */
                        case 0:
                            finish();
                            break;

                        /* Критическая ошибка при отправке запросов */                            
                        case 1:
                            diag_requests_loop();
                            finish();
                            break;
                    }
                /* Не удалось проверить наличие лога */
                case 1:
                    diag_check_log();
                    finish();
                    break;

                /* Нужно создать лог */
                case 2:
                    /* Создание лога */
                    switch (create_log()) {
                        /* Лог успешно создан */
                        case 0: 
                            /* Цикл запросов */
                            switch (requests_loop(ipv4, count, loop)) {
                                /* Конец */
                                case 0:
                                    finish();
                                    break;

                                /* Критическая ошибка при отправке запросов */                            
                                case 1:
                                    diag_requests_loop();
                                    finish();
                                    break;
                            }
                            break;
                    
                        /* Произошла ошибка при создании лога */
                        case 1:
                            diag_create_log();
                            finish();
                            break;
                    }
                    break;
            }
            break;

        /* Аргументы неверные */
        case 1:
            diag_check_args();
            finish();
            break;
    }
    
    return 0;
}
