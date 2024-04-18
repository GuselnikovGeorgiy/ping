#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/statvfs.h>    
#include <string.h>
#include <time.h>

int error_code_log;

void finish_log(void) {
    exit(0);
}

int OpenFileToLog(FILE *file_ptr, const char *path) {
    // Открываем файл для записи ("a" означает режим дозаписи)
    file_ptr = fopen(path, "a");
    if (file_ptr == NULL) {
        error_code_log = 3;
        return 2; // Возвращаем код ошибки
    }
    return 0;
}

// Функция для проверки существования файла
int FileExists(const char *path) {

    struct stat info;

    if (stat(path, &info) != 0) {
        return 0;
    } else {
        return 2;
    }
}

// Функция для проверки места на жестком диске
int DiskSpaceCheck(const char *path) {

    struct statvfs stat;

    if (statvfs(path, &stat) == 0) {
        unsigned long long free_space = stat.f_bsize * stat.f_bfree;
        if (free_space < 1024*1024) {
            error_code_log = 2;
            return 2;
        }
    }

    return 0;
}

int CreateLogFile(FILE *file_ptr, const char *path) {
    // Открываем файл для записи ("a" означает режим дозаписи)
    file_ptr = fopen(path, "a");

    if (file_ptr == NULL) {
        error_code_log = 1;
        return 2; // Возвращаем код ошибки
    }
    return 0;
}

int diag_log() {
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
}
    
    
    

// Функция записи ответа в журнал
int printResult(FILE *file_ptr, const char *path, char* content) {
    if (file_ptr == NULL) {
        time_t rawtime;
        fprintf(file_ptr, "%s %s", ctime(&rawtime), content);
        return 0;
    } else {
        printf("%s\n", content);
        error_code_log = 4;
        return 2;
    }
}


int InitLog(FILE *file_ptr, const char *path) {

    switch (DiskSpaceCheck(path))
    {
    case 0:
        switch (FileExists(path))
        {
        case 0:

            switch (OpenFileToLog(file_ptr, path))
            {
            case 0:
                finish_log();
                break;
            
            case 2:
                diag_log();
                finish_log();
                break;
            }

        case 2:
            switch (CreateLogFile(file_ptr, path))
            {
                case 0:
                    printf("Файл успешно создан: %s\n", path);
                    finish_log();
                    break;
                
                case 2:
                    diag_log();
                    finish_log();
                    break;
            }
            break;
        }
        break;
    
    case 2:
        
        diag_log();
        finish_log();
        break;
    }

    return 0;

}

int WriteLog(FILE *file_ptr, const char* path, char* content) {
    switch (DiskSpaceCheck(path))
    {
    case 0:
        switch (FileExists(path))
        {
        case 0:
            switch (printResult(file_ptr, path, content))
            {
                case 0:
                    finish_log();
                    break;
                
                case 2:
                    diag_log();
                    finish_log();
                    break;
            }
            break;
        
        case 2:
            switch (InitLog(file_ptr, path))
            {
            case 0:
                switch (printResult(file_ptr, path, content))
                {
                case 0:
                    finish_log();
                    break;
                
                case 2:
                    diag_log();
                    finish_log();
                    break;
                }
                break;
            
            case 2:
                finish_log();
                break;
            }
            break;
        }
        break;
    
    case 2:
        diag_log();
        finish_log();
        break;
    }
}
