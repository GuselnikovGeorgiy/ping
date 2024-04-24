#ifndef LOG_H  // Директива условной компиляции для предотвращения повторного включения
#define LOG_H
#include <stdio.h>

// Объявление функции инициализации лога
int init_log(const char *path);

// Объявление функции записи в лог
int write_log(char *path, char *content);

// Объявление функции закрытия лога
int close_log();

#endif  // Завершение директивы условной компиляции
