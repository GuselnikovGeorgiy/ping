//
// ДЕКЛАРАЦИЯ БИБЛИОТЕК
//
#include <stdio.h>            // фукнции ввода/вывода
#include <stdlib.h>           // Cтандартные определения
#include <string.h>           // Cтроковые операции
#include <unistd.h>           // Cтандартные символьные константы и типы
#include <sys/socket.h>       // Основные типы и структуры для работы с сокетами
#include <netinet/in.h>       // Семейство интернет-адресов
#include <netinet/ip.h>       // Дополняет семейство интернет-адресов
#include <netinet/ip_icmp.h>  // Дополняет семейство интернет-адресов типами для icmp прокола
#include <arpa/inet.h>        // Определения интернет операций
#include <netdb.h>            // Определения для операций с сетевойо базой данных
#include <sys/time.h>         // Функции и типы для работы со временем 
#include <signal.h>           // Функции и типы для работы с сигналами
#include <regex.h>            // Функции и типы для работы с регулярными выражениями
#include <ctype.h>            // Функции для работы с типами данных
#include <errno.h>            // Коды ошибок
#include <stdint.h>           // Целочисленные типы
#include <stdbool.h>          // Булевые типы и значения
#include <limits.h>           // Константы определенные реализацией

//
// ДЕКЛАРАЦИЯ КОНСТАНТ
//
#define DEFAULT_PACKET_SIZE 64  // Размер отправляемого пакета по умолчанию в байтах
#define PING_TIMEOUT    2       // Время ожидания на получение одного запроса в секундах
#define DEFAULT_COUNT   4       // Количество запросов по умолчанию
#define DEFAULT_SLEEP_TIME 1    // Задержка между получением запроса и отправки нового в секундах

//
// ДЕКЛАРАЦИЯ ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ
//
int interrupted;  // Флаг прерывания, 1 если пользователь приостановил программу
int count;        // Количество запросов
int loop;         // 1 если неограниченное кол-во запросов иначе 0
char *path;       // Путь до лога
char *ipv4;       // ipv4 введенный пользователем

//
// ДЕКЛАРАЦИЯ ПРОЦЕДУР
//

void sigint_handler(int sigint) // Функция для сигнала остановки
{
    interrupted = 1;
}

int validate_ip(const char *ip) // Функция проверки ipv4 на валидность
{
    regex_t regex;
    int result;
    // Регулярное выражение для проверки ip-адреса
    char *pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"; 

    result = regcomp(&regex, pattern, REG_EXTENDED);    // Компилируем выражение
    if (result) {
        fprintf(stderr, "Невозможно скомпилировать регулярное выражение\n");
        return -1;
    }

    result = regexec(&regex, ip, 0, NULL, 0);           // Сравниваем строку с выражением
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

int is_positive_int(const char *num)            // Функция проверки на целочисленный положительный тип
{
    if (num == NULL || *num == '\0') {          // Если число пустое
        return 1;
    }
    char *endptr;
    long value = strtol(num, &endptr, 10);

    if (errno == ERANGE || value <= 0)          // Если выходит за пределы или неположительное
        return 1;
    if (*endptr == '\0')                        // Если конечный указатель равен концу строки
        return 0;
    return 1;
}

/*
* Параметры функции check_args: 
* int argc - количество аргументов
* char *argv[] - аргументы
*/
int check_args(int argc, char *argv[])          // Функция проверки входных аргументов
{
    // TODO: Добавить поинтеры
    if (argc < 2 || argc > 4) {                 // Проверяем количество поступивших аргументов
        printf("Usage: %s <IPv4> [log_dir] [num_count:int | -t]\n", argv[0]);
        return -1;
    }
    
    int flag_count = 0;
    int flag_loop = 0;
    int flag_log = 0;
    ipv4 = argv[1];

    if (validate_ip(ipv4) == 0) {                // Запускаем проверку ip
        for (int i=2; i < argc; ++i) {           // Запускаем цикл проверки аргументов
            if (strcmp(argv[i], "-t") == 0) {    // Флаг цикла
                if (!flag_loop & !flag_count) {
                    loop = 1;
                    flag_loop = 1;
                    continue;
                }
                else {
                    printf("Недопустимое значение: %s.\n", argv[i]);  // Ошибка при повторном обнаружении
                    return 1;
                }
            } else if (is_positive_int(argv[i]) == 0) {  // Число запросов
                if (!flag_count & !flag_loop) {
                    count = atoi(argv[i]);
                    flag_count = 1;
                    continue;
                } else {
                    printf("Недопустимое значение: %s.\n", argv[i]); // Ошибка при повторном обнаружении
                    return 1;
                }
            } else if (access(argv[i], W_OK) != -1 && access(argv[i], F_OK) != -1) {  // Поиск и проверка на доступность файла для лога
                if (!flag_log) {
                    path = argv[i];
                    flag_log++;
                    continue;
                } else {
                    printf("Недопустимое значение: %s.\n", argv[i]); // Ошибка при повторном обнаружении
                    return 1;
                }
            }
        }
        // Если файл лога был задан, но флаг лога равен 0, то выводим ошибку
        if ((argc > 3 && !flag_log && (flag_count || flag_loop)) || (!flag_log && argc > 2 && (!flag_count && !flag_loop))) {
            printf("Файл недоступен или не существует.\n");
            return 1;
        }
        printf("Write log to: %s\n", path);
        return 0;
    }

    return 1;
}

int diag_check_args() // Функция диагностики проверки аргументов
{
    return 0;
}

int check_log() // Функция проверки наличия лога
{
    return 0;
}

int diag_check_log() // Функция диагностики проверки наличия лога
{
    return 0;
}

int write_log() // Функция записи в лог
{
    return 0;
}

int create_log() // Функция создания лога
{
    return 0;
}

int diag_create_log() // Функция диагностики создания лога
{
    return 0;
}

void finish() // Функция завершения программы
{
    exit(0);
}

unsigned short checksum(void *b, int len) // Функция проверки контрольной суммы ICMP пакета
{   
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

/**
 * Параметры send_request:
 * int sockfd - дескриптор сокета
 * struct sockaddr_in *addr - структура адреса
 * int seq_num - номер запроса
*/
int send_request(int sockfd, struct sockaddr_in *addr, int seq_num) // Функция отправки icmp запроса
{
    // Создаем буфер для ICMP пакета в виде структуры,
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

int receive_response(int sockfd, struct sockaddr_in *addr, int seq_num) // Функция получения icmp запроса
{
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
        return -1;
    }
    
    // Проверка, что ответ пришел не от целевого хоста (крайне маловероятно)
    if (response_addr.sin_addr.s_addr != addr->sin_addr.s_addr) {
        printf("Получено сообщение не от целевого хоста\n");
        return 1;
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

int print_statisctics(int packets_sent, int packets_received, double total_time) // Функция вывода статистики
{
    printf("\n--- Ping statistics ---\n");
    printf("%d packets transmitted, %d received, %.2f packet loss, time %.2fms\n",
           packets_sent, packets_received, 
           ((double)(packets_sent - packets_received) / packets_sent) * 100, total_time);
    
    return 0;
}

int requests_loop() // Функция главного цикла пинга
{
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
    addr.sin_addr.s_addr = inet_addr(ipv4);

    gettimeofday(&start_time, NULL);

    // Устанавливаем таймаут для пинга
    struct timeval timeout;
    timeout.tv_sec = PING_TIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        return 1;
    }

    printf("Pinging %s, with %d bytes of data:\n", ipv4, DEFAULT_PACKET_SIZE);

    while (!interrupted) {
        
        if (packets_sent >= count && !loop) {
            break;
        }

        if (send_request(sockfd, &addr, seq_num) != 0) {
            break;
        }

        switch(receive_response(sockfd, &addr, seq_num)) {
            case 0:
                ++packets_received; // Ответ получен
                break;
            case -1:                // Timeout
                break;
            default:                // Ошибка при получении
                return 1;
        }

        ++seq_num;
        ++packets_sent;

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

int diag_requests_loop() // Функция диагностики цикла запросов
{
    return 0;
}

int main(int argc, char *argv[]) // Главная функция программы
{

    // Инициализация переменных
    count = DEFAULT_COUNT;
    loop = 0;
    path = "";
    ipv4 = "";

    // Тело процедуры
    switch(check_args(argc, argv)) /* Проверка аргументов */ 
    {
        case 0: /* Аргументы верные */
            switch(check_log()) /* Проверка наличия лога */
            { 
                case 0: /* Лог есть, ничего делать не надо */     
                    switch (requests_loop(ipv4, count, loop))
                    {
                        case 0: /* Конец */
                            finish();
                            break;

                        case 1: /* Критическая ошибка при отправке запросов */
                            diag_requests_loop();
                            finish();
                            break;
                    }
                
                case 1: /* Не удалось проверить наличие лога */
                    diag_check_log();
                    finish();
                    break;

                case 2: /* Нужно создать лог */
                    switch (create_log()) /* Создание лога */
                    {
                        case 0: /* Лог успешно создан */
                            switch (requests_loop(ipv4, count, loop)) /* Цикл запросов */
                            {
                                case 0: /* Конец */
                                    finish();
                                    break;

                                case 1: /* Критическая ошибка при отправке запросов */
                                    diag_requests_loop();
                                    finish();
                                    break;
                            }
                            break;
                
                        case 1: /* Произошла ошибка при создании лога */
                            diag_create_log();
                            finish();
                            break;
                    }
                    break;
            }
            break;

        case 1: /* Аргументы неверные */
            diag_check_args();
            finish();
            break;
    }
    
    return 0;
}
