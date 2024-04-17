//
// ДЕКЛАРАЦИЯ БИБЛИОТЕК
//
#include <stdio.h>            // Фукнции ввода/вывода
#include <stdlib.h>           // Функции выделения памяти
#include <string.h>           // Cтроковые операции
#include <unistd.h>           // Для работы с POSIX
#include <sys/socket.h>       // Для работы с сокетами
#include <netinet/in.h>       // Семейство интернет-адресов
#include <netinet/ip.h>       // Для работы с IP
#include <netinet/ip_icmp.h>  // Для работы с ICMP
#include <arpa/inet.h>        // Определения интернет операций
#include <netdb.h>            // Определения для операций с сетевой базой данных
#include <sys/time.h>         // Функции и типы для работы со временем 
#include <signal.h>           // Функции и типы для работы с сигналами
#include <regex.h>            // Функции и типы для работы с регулярными выражениями
#include <errno.h>            // Коды ошибок
#include <stdint.h>           // Целочисленные типы
#include <limits.h>           // Константы лимитов

//
// ДЕКЛАРАЦИЯ ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ
//
int DEFAULT_PACKET_SIZE;      // Размер отправляемого пакета по умолчанию в байтах
int PING_TIMEOUT;             // Время ожидания на получение одного запроса в секундах
int DEFAULT_SLEEP_TIME;       // Задержка между получением запроса и отправки нового в секундах
int count;                    // Количество запросов
int loop;                     // 1 если неограниченное кол-во запросов, иначе 0
char *path;                   // Путь до лога
char *ipv4;                   // ipv4 введенный пользователем
int sockfd;                   // Дескриптор сокета
struct sockaddr_in addr;      // Адрес
struct timeval start_time;    // Стартовой время отправки запросов
int packets_sent;             // Кол-во отправленных пакетов
int packets_received;         // Кол-во полученных пакетов

//
// ДЕКЛАРАЦИЯ ПРОЦЕДУР
//

// TODO: заменить на ориг лог функции
int init_log() // Функция инициализации лога
{
    return 0;
}

int write_log() // Функция записи в лог
{
    return 0;
}

int close_log() // Функция закрытия лога
{
    return 0;
}

void finish() // Функция завершения программы
{
    // printf("Вход в finish\n");                   // DEBUG 

    close(sockfd);
    close_log();

    // printf("Выход из finish, 0\n");              // DEBUG 
    exit(0);
}


int validate_ip(const char *ip) // Функция проверки ipv4 на валидность
{
    // printf("Вход в validate_ip\n");                   // DEBUG

    regex_t regex;  // pattern buffer
    int result;     // Результат
    char *pattern;  // regex pattern

    // Регулярное выражение для проверки ip-адреса
    pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"; 

    result = regcomp(&regex, pattern, REG_EXTENDED);     // Компилируем выражение
    if (result) {
        fprintf(stderr, "Невозможно скомпилировать регулярное выражение\n");
        return -1;
    }

    result = regexec(&regex, ip, 0, NULL, 0);            // Сравниваем строку с выражением
    regfree(&regex);

    if (!result) {
        // printf("Выход из validate_ip, 0\n");          // DEBUG
        return 0; // соответствует IPv4
    } else if (result == REG_NOMATCH) {
        // printf("Выход из validate_ip, 1\n");          // DEBUG
        printf("Задан неверный IP\n");
        return 1;
    } else {
        // printf("Выход из validate_ip, -1\n");         // DEBUG
        fprintf(stderr, "Ошибка при работе с регулярным выражением\n");
        return -1;
    }
}

int is_positive_int(const char *num)                  // Функция проверки на целочисленный положительный тип
{
    // printf("Вход в is_positive_int\n");            // DEBUG

    char *endptr;
    long value;

    if (num == NULL || *num == '\0') {                // Если число пустое
        // printf("Выход из is_positive_int, 1\n");   // DEBUG
        return 1;
    }
    
    strtol(num, &endptr, 10);

    if (errno == ERANGE || value <= 0)                // Если выходит за пределы или неположительное
        // printf("Выход из is_positive_int, 1\n");   // DEBUG
        return 1;
    if (*endptr == '\0')                              // Если конечный указатель равен концу строки
        // printf("Выход из is_positive_int, 0\n");   // DEBUG
        return 0;

    // printf("Выход из is_positive_int, 1\n");       // DEBUG
    return 1;
}

int check_args(int argc, char *argv[])                // Функция проверки входных аргументов
{
    // printf("Вход в check_args\n");                 // DEBUG

    // int argc - количество аргументов
    // char *argv[] - аргументы

    if (argc < 2 || argc > 4) {                       // Проверяем количество поступивших аргументов
        printf("Usage: %s <IPv4> [log_dir] [num_count:int | -t]\n", argv[0]);
        // printf("Выход из check_args, -1\n");       // DEBUG
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
                    // printf("Выход из check_args, 1\n");       // DEBUG
                    return 1;
                }
            } else if (is_positive_int(argv[i]) == 0) {  // Число запросов
                if (!flag_count & !flag_loop) {
                    count = atoi(argv[i]);
                    flag_count = 1;
                    continue;
                } else {
                    printf("Недопустимое значение: %s.\n", argv[i]); // Ошибка при повторном обнаружении
                    // printf("Выход из check_args, 1\n");       // DEBUG
                    return 1;
                }
            } else if (access(argv[i], W_OK) != -1 && access(argv[i], F_OK) != -1) {  // Поиск и проверка на доступность файла для лога
                if (!flag_log) {
                    path = argv[i];
                    flag_log++;
                    continue;
                } else {
                    printf("Недопустимое значение: %s.\n", argv[i]); // Ошибка при повторном обнаружении
                    // printf("Выход из check_args, 1\n");       // DEBUG
                    return 1;
                }
            }
        }
        // Если файл лога был задан, но флаг лога равен 0, то выводим ошибку
        if ((argc > 3 && !flag_log && (flag_count || flag_loop)) || (!flag_log && argc > 2 && (!flag_count && !flag_loop))) {
            printf("Файл недоступен или не существует.\n");
            // printf("Выход из check_args, 1\n");               // DEBUG
            return 1;
        }
        printf("Write log to: %s\n", path);
        // printf("Выход из check_args, 0\n");                   // DEBUG
        return 0;
    }

    // printf("Выход из check_args, 0\n");                       // DEBUG
    return 1;
}

int diag_check_args() // Функция диагностики проверки аргументов
{
    // printf("Вход в diag_check_args\n")                        // DEBUG 
    // printf("Выход из diag_check_args, 0\n")                   // DEBUG 
    return 0;
}

unsigned short checksum(void *b, int f_len) // Функция проверки контрольной суммы ICMP пакета
{   
    // printf("Вход в checksum\n");                              // DEBUG 

    // Объявление переменных
    unsigned short *buf;     // Буфер
    int len;                 // Длина буфера
    unsigned int sum;        // Промежуточная сумма
    unsigned short result;   // Результат

    // Инициализация переменных
    buf = b;
    len = f_len;
    sum = 0;        
    result = 0;

    // Тело процедуры
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;

    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    // printf("Выход из checksum, 0\n");                          // DEBUG 
    return result;
}

int send_request(int f_seq_num) // Функция отправки ICMP запроса
{
    // printf("Вход в send_request\n");                           // DEBUG 

    // Объявление переменных
    int seq_num;                      // Порядковый номер пакета
    char packet[DEFAULT_PACKET_SIZE]; // Буфер для ICMP пакета
    struct icmp *icmp_packet;         // ICMP пакет

    // Инициализация переменных
    seq_num = f_seq_num;

    // Тело процедуры
    if (packets_sent >= count && !loop) {
        // printf("Выход из send_request, 2\n");                   // DEBUG 
        return 2;
    }

    memset(packet, 0, DEFAULT_PACKET_SIZE);
    icmp_packet = (struct icmp *)packet;
    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_id = getpid(); // pid процесса
    icmp_packet->icmp_seq = seq_num;
    icmp_packet->icmp_cksum = 0;

    // Временные метки
    gettimeofday((struct timeval *)icmp_packet->icmp_data, NULL);

    // Считаем контрольную сумму пакет
    icmp_packet->icmp_cksum = checksum((unsigned short *)icmp_packet, DEFAULT_PACKET_SIZE);

    // Отправляем запрос
    if (sendto(sockfd, packet, DEFAULT_PACKET_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Ошибка отправки запроса к адресу: send_request");
        // printf("Выход из send_request, 1\n");                   // DEBUG 
        return 1;
    }

    // printf("Выход из send_request, 0\n");                       // DEBUG 
    return 0;
}

int diag_send_request() // Функция диагностики отправления запроса
{
    return 0;
}

int receive_response(int f_seq_num) // Функция получения icmp запроса
{
    // printf("Вход в receive_response\n");                        // DEBUG 

    // Объявление переменных
    int seq_num;                       // Порядковый номер пакета
    char buffer[DEFAULT_PACKET_SIZE];  // Буфер
    struct sockaddr_in response_addr;  // Адрес
    socklen_t response_addr_len;       // Размер адреса
    int bytes_received;                // Кол-во полученных байтов
    struct iphdr *ip_header;           // ICMP заголовок
    struct icmp *icmp_packet;          // ICMP пакет

    // Инициализация переменных
    seq_num = f_seq_num;
    response_addr_len = sizeof(response_addr);
    bytes_received = 0;

    // Тело процедуры
    memset(buffer, 0, DEFAULT_PACKET_SIZE);

    // Получение ответа от хоста
    bytes_received = recvfrom(sockfd, buffer, DEFAULT_PACKET_SIZE, 0, (struct sockaddr *)&response_addr, &response_addr_len);

    if (bytes_received < 0) {
        printf("Request timed out...\n");
        // printf("Выход из receive_response, 1\n");               // DEBUG 
        return 1;
    }
    
    // Проверка, что ответ пришел не от целевого хоста (крайне маловероятно)
    if (response_addr.sin_addr.s_addr != addr.sin_addr.s_addr) {
        printf("Получено сообщение не от целевого хоста\n");
        // printf("Выход из receive_response, 1\n");               // DEBUG 
        return 1;
    }

    // Получаем заголовки для IP/ICMP
    ip_header = (struct iphdr *)buffer;
    icmp_packet = (struct icmp *)(buffer + (ip_header->ihl << 2));
    
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
        // printf("Выход из receive_response, 1\n");               // DEBUG 
        return 1;
    }

    // printf("Выход из receive_response, 0\n");                   // DEBUG 
    return 0;
}

int print_statisctics() // Функция вывода статистики
{
    // printf("Вход в print_statisctics\n")                        // DEBUG 

    // Объявление переменных 
    double total_time;        // Общее время выполнения всех запросов
    struct timeval end_time;  // Конечное время отправки всех запросов

    // Инициализация переменных
    total_time = 0;

    // Тело процедуры
    gettimeofday(&end_time, NULL);
    total_time = (double)(end_time.tv_sec - start_time.tv_sec) * 1000 +
                 (double)(end_time.tv_usec - start_time.tv_usec) / 1000;

    printf("\n--- Ping statistics ---\n");
    printf("%d packets transmitted, %d received, %.2f packet loss, time %.2fms\n",
           packets_sent, packets_received, 
           ((double)(packets_sent - packets_received) / packets_sent) * 100, total_time);
    
    // printf("Выход из print_statisctics, 0\n");                  // DEBUG 
    return 0;
}

void sigint_handler() // Функция для сигнала принудительного выхода из программы
{
    print_statisctics();
    finish();
}

int init_socket() // Функция инициализации сокета
{
    // printf("Вход в init_socket\n")                              // DEBUG 
    struct timeval timeout; // Таймаут для пинга

    signal(SIGINT, sigint_handler);

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket");
        // printf("Выход из init_socket, 1\n")                     // DEBUG 
        return 1;
    }

    // Структура адреса
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipv4);

    // Устанавливаем таймаут для пинга
    timeout.tv_sec = PING_TIMEOUT;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        // printf("Выход из init_socket, 1\n");                    // DEBUG 
        return 1;
    }

    gettimeofday(&start_time, NULL);

    printf("Pinging %s, with %d bytes of data:\n", ipv4, DEFAULT_PACKET_SIZE);

    // printf("Выход из init_socket, 0\n");                        // DEBUG 
    return 0;
}

int diag_init_socket() // Функция диагностики инициализации сокета
{
    return 0;
}

//
//ТЕЛО ПРОГРАММЫ
//
int main(int argc, char *argv[]) // Главная функция программы
{
    // printf("Вход в main\n");                                    // DEBUG 

    // Объявления переменных
    int seq_num;            // Порядковый номер пакета

    // Инициализация переменных
    DEFAULT_PACKET_SIZE = 64;
    PING_TIMEOUT = 2;
    DEFAULT_SLEEP_TIME = 1;
    count = 4;
    loop = 0;
    path = "";
    ipv4 = ""; 
    seq_num = 0;
    packets_sent = 0;
    packets_received = 0;

    // Тело процедуры
    switch(check_args(argc, argv))                                 /* Проверка аргументов */ 
    {
        case 0:                                                    /* Аргументы верные */ 
            switch(init_log())                                     /* Инициализация лога */ 
            {
                case 0:                                            /* Лог инициализировался */ 
                    switch(init_socket())                          /* Инициализация сокета */  
                    {
                        case 0:                                    /* Сокет успешно создался */
                            while (1)                              /* Цикл с запросами начался */
                            {
                                switch(send_request(seq_num))  /* Отправка запроса */ 
                                {
                                    case 2:                        /* Завершаем цикл */  
                                        print_statisctics();
                                        finish();
                                        break;

                                    case 0:                        /* Запрос успешно отправился */
                                        switch(receive_response(seq_num)) /* Получение ответа */
                                        {
                                            case 0:                /* Ответ успешно получен */ 
                                                ++packets_received;
                                                break;
                                                
                                            case 1:                /* Ответ не получен */ 
                                                write_log();
                                                break;
                                        }
                                        break;

                                    case 1:                        /* Запрос не отправился */
                                        diag_send_request();
                                        print_statisctics();
                                        write_log();
                                        finish();
                                        break;
                                }

                                ++seq_num;
                                ++packets_sent;
                                sleep(DEFAULT_SLEEP_TIME);
                            }
                            break;

                        case 1:                                    /* Ошибка с сокетом */    
                            diag_init_socket();
                            finish();
                            break;
                    }
                    break;

                case 2:                                            /* Лог не инициализировался */    
                    finish();
                    break;        
            }
            break;

        case 1:                                                    /* Неверные аргументы */ 
            diag_check_args();

            switch(init_log())                                     /* Инициализация лога */ 
            {
                case 0:                                            /* Лог успешно инициализирован */  
                    write_log();
                    finish();
                    break;

                case 2:                                            /* Лог не был инициализирован */  
                    finish();
                    break;        
            }
            break;
    }
    
    // printf("Выход из main\n");                                  // DEBUG
    return 0;
}
