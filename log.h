#ifndef LOG_H  // Директива условной компиляции для предотвращения повторного включения
#define LOG_H
#include <stdio.h>

// Объявление функции 1
int InitLog(FILE *file_ptr, const char *path);

// Объявление функции 2
int WriteLog(FILE *file_ptr, const char *path, char *content);

#endif  // Завершение директивы условной компиляции