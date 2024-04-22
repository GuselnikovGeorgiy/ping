//
// ДЕКЛАРАЦИЯ БИБЛИОТЕК
//
#include <stdio.h>       // Стандартные ввода/вывода
#include <stdlib.h>      // Стандартные функции общего назначения: выделение памяти, конверсия типов и т.д.
#include <sys/stat.h>    // Функции для работы с информацией о файле (например, проверка существования файла)
#include <sys/statvfs.h> // Функции для работы с информацией о файловой системе (например, проверка места на диске)
#include <string.h>      // Функции для работы со строками (например, strcmp, strlen)
#include <time.h>        // Функции для работы со временем и датой (например, получение текущего времени)
//
// ДЕКЛАРАЦИЯ ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ
//

int error_code_log;      // Вывод ошибок в терминал
bool barrier;            // Проверка вхождения в архивацию

//
// ДЕКЛАРАЦИЯ ПРОЦЕДУР
//

int CloseLog() // Закрытие файла лога
{
    FILE *file; // Объявление поинтера на файл

    file = NULL; // Инициализация file

    // printf("Вход в CloseLog\n");                         // DEBUG
    fclose(file);

    if (file == NULL) 
    {
        print("Не удалось закрыть файл.");
        int error_code_log = 2;
        return 2;
    }
    // printf("Выход из CloseLog\n");                       // DEBUG
    return 0;

}

int SizeCheck(path)                                                                    // Проверка размера лога и дальнейшая архивация при размере больше 20 МБ
{
    // Декларация переменных
    FILE *file;                                                                        // Объявление поинтера на файл
    char file_name[1024];                                                              // Массив символов для хранения имени файла
    struct statvfs fs_info;                                                            // Структура для хранения информации о файловой системе
    long long size;                                                                    // Переменная для хранения размера файла в байтах

    //Инициализация перменных
    FILE *file = NULL;                                                                 // Инициализация указателя на файл
    char file_name[1024] = "";                                                         // Инициализация массива символов для хранения имени файла
    struct statvfs fs_info = {};                                                       // Инициализация структуры для хранения информации о файловой системе
    long long size = 0;                                                                // Инициализация переменной для хранения размера файла в байтах

    // printf("Вход в SizeCheck\n");                                                   // DEBUG

    file = fopen(path) 
    struct statvfs fs_info;

    if (file == NULL) 
    {
        error_code_log = 2;
        printf("Не удалось открыть файл.");
        return 2;
    }

    if (fscanf(file, "%s", file_path) != 1) 
    {
        printf("Ошибка чтения пути до файла.");
        fclose(file);
        error_code_log = 2;
        return 2;
    }
     
     fclose(file);

     if (statvfs(file, &fs_info) != 0) 
     {
        printf("Ошибка получения информации о файловой системе.");
        error_code_log = 2;
        return 2;
     }

     long long size = (long long)fs_info.f_frsize * fs_info.f_blocks;

     if (size >= 1024 * 1024 * 20) 
     {
        system("python3 log_zip.py");
     }
     
    // printf("Выход из SizeCheck\n");                                                // DEBUG

     return(0);

}

int OpenFileToLog(FILE *ptr, const char *p)                                   // Открытие файла для записи в лог
{
    // Декларация переменных
    FILE *file_ptr;                                                                   // Переменная поинтера на файл
    const char *path;                                                                 // Переменная поинтера на строку

    // Инициализация переменных
    file_ptr = ptr;                                                                   // Инициализация указателя на файл значением NULL
    path = p;                                                                         // Инициализация поинтера на строку переданным значением

    // printf("Вход в OpenFileToLog\n");                                              // DEBUG
    file_ptr = fopen(path, "a");
    if (file_ptr == NULL) 
    {
        error_code_log = 3;
        // printf("Выход из OpenFileToLog, 2\n");                                     // DEBUG
        return 2;                                                                     // Возвращаем код ошибки
    }
    // printf("Выход из OpenFileToLog, 0\n");                                         // DEBUG
    return 0;
}

int FileExists(const char *p)                                                         // Проверка существования файла
{
    // Декларация переменных
    const char *path;                                                                // Переменная поинтера на строку
    struct stat info;                                                                // Переменная структуры для хранения информации о файле

    //Инициализация переменных
    info = {};                                                                       // Инициализация структуры нулевыми значениями
    path = p;                                                                        // Инициализация поинтера на строку переданным значением

    // printf("Вход в FileExists\n");                                                // DEBUG
    if (stat(path, &info) != 0) 
    {
        // printf("Выход из FileExists, 0\n");                                       // DEBUG
        return 0;                                                                    // Файл не существует
    } else 
    {
        // printf("Выход из FileExists, 2\n");                                       // DEBUG
        return 2;                                                                    // Файл существует
    }
}

int DiskSpaceCheck(const char *p)                                                 // Проверка места на жестком диске
{
    // Декларация переменных
    struct statvfs stat;                                                             // Структура для хранения информации о файловой системе
    unsigned long long free_space;                                                   // Размер свободного пространства на диске
    char archive[100];                                                               // Массив символов для хранения команды архивации файла
    const char *path;                                                                // Переменная поинтера на строку

    // Инициализация переменных
    stat = {};                                                                       // Инициализация структуры для хранения информации о файловой системе
    free_space = 0;                                                                  // Инициализация переменной для хранения размера свободного пространства на диске
    archive[100] = "";                                                               // Инициализация массива символов для хранения команды архивации файла
    path = p;                                                                        // Инициализация поинтера на строку переданным значением

    // printf("Вход в DiskSpaceCheck\n");                                           // DEBUG
    struct statvfs stat;
    if (statvfs(path, &stat) == 0)                                                  // Функция statvfs возвращает 0 при успешном выполнении
    {                                                                               
        unsigned long long free_space = stat.f_bsize * stat.f_bfree;                // Вычисляем доступное место
        if ((free_space < 1024*1024) & (barrier == false))                          // Если свободное место меньше 1 МБ
        {                        

            char archive[100];
            sprintf(archive, "python3 log_zip.py %d", path);
            system(archive);                                                        // Освобождение места архивацией внутренних файлов
            DiskSpaceCheck(path);                                                   // Перезапуск проверки места
            barrier = true;                                                         // Изменение состояния барьера                                          

        } elif ((free_space < 1024*1024) & (barrier == true))
        {
            
            error_code_log = 2;                                                     // Устанавливаем код ошибки
            barrier = false;                                                        // Возврат состояния барьера
            return 2;                                                               // Возвращаем код ошибки

        }
    }                                                                               // Возврат состояния барьера
    // printf("Выход из DiskSpaceCheck, 0\n");                                      // DEBUG
    barrier = false; 
    return 0;
}

int CreateLogFile(FILE *ptr, const char *p)                                         // Создание лог-файла
{
    // Декларация переменных
    FILE *file_ptr;                                                                 // Переменная поинтера на файл
    const char *path;                                                               // Переменная поинтера на строку

    // Инициализация переменных
    file_ptr = ptr;                                                                 // Инициализация указателя на файл значением NULL
    path = p;                                                                       // Инициализация поинтера на строку переданным значением

    // printf("Вход в CreateLogFile\n");                                            // DEBUG
    file_ptr = fopen(path, "a");
    if (file_ptr == NULL) 
    {
        error_code_log = 1;
        // printf("Выход из CreateLogFile, 2\n");                                   // DEBUG
        return 2;                                                                   // Возвращаем код ошибки
    }
    // printf("Выход из CreateLogFile, 0\n");                                       // DEBUG
    return 0;
}

int diag_log() // Вывод диагностического сообщения
{
    // printf("Вход в diag_log\n");                                                 // DEBUG
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
    // printf("Выход из diag_log\n");                                              // DEBUG
    return 0;
}

int printResult(FILE *ptr, const char *p, char* c)                                 // Запись ответа в журнал
{
    // Декларация переменных
    FILE *file_ptr;                                                                // Переменная указателя на файл
    time_t rawtime;                                                                // Переменная для хранения текущего времени
    const char *content;                                                           // Переменная указателя на строку
    const char *path;                                                              // Переменная поинтера на строку

    //Инициализация переменных
    file_ptr = ptr;                                                                // Инициализация указателя на файл значением NULL
    rawtime = time(NULL);                                                          // Инициализация переменной для хранения текущего времени
    content = c;                                                                   // Инициализация указателя на строку
    path = p;                                                                      // Инициализация поинтера на строку переданным значением

    // printf("Вход в printResult\n");                                             // DEBUG
    if (file_ptr == NULL) 
    {
        time_t rawtime;
        fprintf(file_ptr, "%s %s", ctime(&rawtime), content);
        // printf("Выход из printResult, 0\n");                                    // DEBUG
        return 0;
    } else 
    {
        printf("%s\n", content);
        error_code_log = 4;
        // printf("Выход из printResult, 2\n");                                    // DEBUG
        return 2;
    }
}

int InitLog(const char *p) // Инициализация лога (проверка диска, существование файла, открытие файла)
{
    // Декларация переменных
    const char *path;                                                              // Переменная поинтера на строку

    //Инициализация переменных
    path = p;                                                                      // Инициализация поинтера на строку переданным значением

    // printf("Вход в InitLog\n");                          // DEBUG
    switch (DiskSpaceCheck(path)) 
    {
    case 0:
        switch (FileExists(path)) 
        {
        case 0:
            switch (OpenFileToLog(file_ptr, path)) 
            {
            case 0:
                // printf("Выход из InitLog, 0\n");      // DEBUG
                return 0;
            case 2:
                diag_log();
                // printf("Выход из InitLog, 2\n");      // DEBUG
                return 2;
            }
        case 2:
            switch (CreateLogFile(file_ptr, path)) 
            {
            case 0:
                printf("Файл успешно создан: %s\n", path);
                // printf("Выход из InitLog, 0\n");      // DEBUG
                return 0;
            case 2:
                diag_log();
                // printf("Выход из InitLog, 2\n");      // DEBUG
                return 2;
            }
            break;
        }
        break;
    case 2:
        diag_log();
        // printf("Выход из InitLog, 2\n");              // DEBUG
        return 2;
    }
    // printf("Выход из InitLog, 0\n");                  // DEBUG
    return 0;
}

int WriteLog(char* c) // Запись в лог (проверка диска, существование файла, запись в файл)
{
    // Декларация переменных
    const char *content;                                                           // Переменная указателя на строку

    // Инициализация переменных
    content = с;                                                                   // Инициализация указателя на строку значением NULL

    // printf("Вход в WriteLog\n");                        // DEBUG
    switch (DiskSpaceCheck(path)) 
    {
    case 0:
        switch (FileExists(path)) 
        {
        case 0:
            switch (printResult(file_ptr, path, content)) 
            {
            case 0:
                // printf("Выход из WriteLog, 0\n");     // DEBUG
                return 0;
            case 2:
                diag_log();
                // printf("Выход из WriteLog, 2\n");     // DEBUG
                return 2;
            }
            break;
        case 2:
            switch (InitLog(file_ptr, path)) 
            {
            case 0:
                switch (printResult(file_ptr, path, content)) 
                {
                case 0:
                    // printf("Выход из WriteLog, 0\n"); // DEBUG
                    return 0;
                case 2:
                    diag_log();
                    // printf("Выход из WriteLog, 2\n"); // DEBUG
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
        // printf("Выход из WriteLog, 2\n");             // DEBUG
        return 2;
    }
    // printf("Выход из WriteLog, 0\n");                 // DEBUG
    return 0;
}