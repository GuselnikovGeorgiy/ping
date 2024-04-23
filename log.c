//
// ДЕКЛАРАЦИЯ БИБЛИОТЕК
//
#include <stdio.h>                                                                       // Стандартные ввода/вывода
#include <stdlib.h>                                                                      // Стандартные функции общего назначения: выделение памяти, конверсия типов и т.д.
#include <sys/stat.h>                                                                    // Функции для работы с информацией о файле (например, проверка существования файла)
#include <sys/statvfs.h>                                                                 // Функции для работы с информацией о файловой системе (например, проверка места на диске)
#include <string.h>                                                                      // Функции для работы со строками (например, strcmp, strlen)
#include <time.h>                                                                        // Функции для работы со временем и датой (например, получение текущего времени)

//
// ДЕКЛАРАЦИЯ ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ
//
int error_code_log;                                                                      // Вывод ошибок в терминал
FILE *file;                                                                              // Поинтер на файл
char* log_path;                                                                          // Путь до файла лога

//
// ДЕКЛАРАЦИЯ ПРОЦЕДУР
//
int close_log()                                                                          // Закрытие файла лога
{
    // printf("Вход в close_log\n");                                                     // DEBUG

    if (fclose(file) != 0)
    {
        printf("Не удалось закрыть файл.");
        return 2;        
    }

    // printf("Выход из close_log\n");                                                   // DEBUG
    return 0;

}

int open_log_file()                                                                      // Открытие файла для записи в лог
{
    // printf("Вход в open_file_to_log\n");                                              // DEBUG
    file = fopen(log_path, "a");
    if (file == NULL) 
    {
        error_code_log = 3;
        // printf("Выход из open_file_to_log, 2\n");                                     // DEBUG
        return 2;                                                                        // Возвращаем код ошибки
    }
    // printf("Выход из open_file_to_log, 0\n");                                         // DEBUG
    return 0;
}

int file_exists()                                                                        // Проверка существования файла
{
    // Декларация переменных
    struct statvfs info;                                                                 // Переменная структуры для хранения информации о файле

    //Инициализация переменных
    // info = {};                                                                        // Инициализация структуры нулевыми значениями

    // printf("Вход в file_exists\n");                                                   // DEBUG
    if (statvfs(log_path, &info) != 0) 
    {
        // printf("Выход из file_exists, 0\n");                                          // DEBUG
        return 0;                                                                        // Файл не существует
    } else 
    {
        // printf("Выход из file_exists, 2\n");                                          // DEBUG
        return 2;                                                                        // Файл существует
    }
}

int disk_space_check()                                                                   // Проверка места на жестком диске
{
    // Декларация переменных
    struct statvfs stat;                                                                 // Структура для хранения информации о файловой системе
    unsigned long long free_space;                                                       // Размер свободного пространства на диске
    char archive[100];                                                                   // Массив символов для хранения команды архивации файла
    int barrier;            // Проверка вхождения в архивацию
    unsigned long long size;

    // Инициализация переменных
    // stat = {};                                                                        // Инициализация структуры для хранения информации о файловой системе
    free_space = 0;                                                                      // Инициализация переменной для хранения размера свободного пространства на диске
    // archive = "";                                                                     // Инициализация массива символов для хранения команды архивации файла
    barrier = 0;
    size = 0;

    // printf("Вход в disk_space_check\n");                                              // DEBUG
    if(size >= 1024 * 1024 * 20) {
        sprintf(archive, "python3 log_zip.py %s", log_path);
        system(archive);                                                 
        barrier = 1;
    }
    
    if (statvfs(log_path, &stat) == 0)                                                   // Функция statvfs возвращает 0 при успешном выполнении
    {                                                                               
        free_space = stat.f_bsize * stat.f_bfree;                                        // Вычисляем доступное место
        if ((free_space < 1024*1024) & (barrier == 0))                                   // Если свободное место меньше 1 МБ
        {                        
            sprintf(archive, "python3 log_zip.py %s", log_path);
            system(archive);                                                             // Освобождение места архивацией внутренних файлов
            disk_space_check(log_path);                                                  // Перезапуск проверки места
            barrier = 1;                                                                 // Изменение состояния барьера                                          

        } else if((free_space < 1024*1024) & (barrier == 1))
        {
            
            error_code_log = 2;                                                          // Устанавливаем код ошибки
            barrier = 0;                                                                 // Возврат состояния барьера
            // printf("Выход из disk_space_check, 2\n");                                 // DEBUG
            return 2;                                                                    // Возвращаем код ошибки

        }
    }                                                                                    // Возврат состояния барьера
    barrier = 0; 
    // printf("Выход из disk_space_check, 0\n");                                         // DEBUG
    return 0;
}

int create_log_file()                                                                    // Создание лог-файла
{
    // printf("Вход в create_log_file\n");                                               // DEBUG
    file = fopen(log_path, "a");
    if (file == NULL) 
    {
        error_code_log = 1;
        // printf("Выход из create_log_file, 2\n");                                      // DEBUG
        return 2;                                                                        // Возвращаем код ошибки
    }
    // printf("Выход из create_log_file, 0\n");                                          // DEBUG
    return 0;
}

int diag_log()                                                                           // Вывод диагностического сообщения
{
    // printf("Вход в diag_log\n");                                                      // DEBUG
    switch (error_code_log) 
    {
        case 1:
            printf("Ошибка при создании файла.\n");
            break;
        case 2:
            printf("Недостаточно места на диске для записи лога.\n");
            break;
        case 3:
            printf("Ошибка при открытии файла.\n");
            break;
        case 4:
            printf("Не удалось записать в лог.\n");
            break;
    }
    // printf("Выход из diag_log, 0\n");                                                 // DEBUG
    return 0;
}

int print_result(const char* c)                                                          // Запись ответа в журнал
{
    // Декларация переменных
    time_t rawtime;
    struct tm *local_time;                                                               // Переменная для хранения текущего времени
    const char *content;                                                                 // Переменная указателя на строку

    //Инициализация переменных
    rawtime = time(NULL);
    local_time = localtime(&rawtime);                                                    // Инициализация переменной для хранения текущего времени
    content = c;                                                                         // Инициализация указателя на строку
    
    // printf("Вход в print_result\n");                                                  // DEBUG
    if (file != NULL) 
    {
        fprintf(file, "%04d-%02d-%02d %02d:%02d:%02d %s\n", 
        local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
        local_time->tm_hour, local_time->tm_min, local_time->tm_sec, content);

        // printf("Выход из print_result, 0\n");                                         // DEBUG
        return 0;
    } else 
    {
        printf("%s\n", content);
        error_code_log = 4;
        // printf("Выход из print_result, 2\n");                                         // DEBUG
        return 2;
    }
}

int init_log(char *p) // Инициализация лога (проверка диска, существование файла, открытие файла)
{
    // Иниаицлизация глобальных переменных
    error_code_log = -1;
    file = NULL; 
    log_path = p;

    // printf("Вход в init_log\n");                                                      // DEBUG
    switch (disk_space_check()) 
    {
    case 0:
        switch (file_exists()) 
        {
        case 0:
            switch (open_log_file()) 
            {
            case 0:
                // printf("Выход из init_log, 0\n");                                     // DEBUG
                return 0;
            case 2:
                diag_log();
                // printf("Выход из init_log, 2\n");                                     // DEBUG
                return 2;
            }
        case 2:
            switch (create_log_file()) 
            {
            case 0:
                // printf("Файл успешно создан: %s\n", log_path);
                // printf("Выход из init_log, 0\n");                                     // DEBUG
                return 0;
            case 2:
                diag_log();
                // printf("Выход из init_log, 2\n");                                     // DEBUG
                return 2;
            }
            break;
        }
        break;
    case 2:
        diag_log();
        // printf("Выход из init_log, 2\n");                                             // DEBUG
        return 2;
    }
    // printf("Выход из init_log, 0\n");                                                 // DEBUG
    return 0;
}

int write_log(char *p, const char* cont) // Запись в лог (проверка диска, существование файла, запись в файл)
{
    // printf("Вход в write_log\n");                                                     // DEBUG
    // Декларация переменных
    const char *content;                                                                 // Переменная указателя на строку

    // Инициализация переменных
    content = cont;                                                                      // Инициализация указателя на строку значением NULL
    log_path = p;

    if (*content == '\0' || content == NULL)
        return 0;

    switch (disk_space_check()) 
    {
    case 0:
        switch (file_exists()) 
        {
        case 0:
            switch (print_result(content)) 
            {
            case 0:
                // printf("Выход из write_log, 0\n");                                    // DEBUG
                return 0;
            case 2:
                diag_log();
                // printf("Выход из write_log, 2\n");                                    // DEBUG
                return 2;
            }
            break;
        case 2:
            switch (init_log(log_path)) 
            {
            case 0:
                switch (print_result(content)) 
                {
                case 0:
                    // printf("Выход из write_log, 0\n"); // DEBUG
                    return 0;
                case 2:
                    diag_log();
                    // printf("Выход из write_log, 2\n"); // DEBUG
                    return 2;
                }
                break;
            case 2:
                return 2;
            }
            break;
        }
        break;
    case 2:
        diag_log();
        // printf("Выход из write_log, 2\n");                                            // DEBUG
        return 2;
    }

    // printf("Выход из write_log, 0\n");                 // DEBUG
    return 0;
}
