//
// ДЕКЛАРАЦИЯ БИБЛИОТЕК
//
#include <stdio.h>            // фукнции ввода/вывода
#include <stdlib.h>           // Cтандартные определения
#include <string.h>           // Cтроковые операции
#include <time.h>             // Функции работы с датой и временем

const char* logName = "logfile.txt";

// Функция обработки файла журнала
int checkLog() {
    FILE* f1 = fopen(logName, "a");
    if (f1 == NULL) {
        return 1;
    }
    fclose(f1);
    return 0;
}

// Функция открытия файла журнала
int openLog() {
    FILE* f1 = fopen(logName, "a");
    if (f1 != NULL) {
        fclose(f1);
        return 0;
    } else {
        return 1;
    }
}

// Функция создания файла журнала
int createLog() {
    FILE* f1 = fopen(logName, "a");
    if (f1 != NULL) {
        fclose(f1);
        return 0;
    } else {
        return 1;
    }
}

// Функция диагностики открытия файла журнала
void diagOpenLog() {
    printf("Ошибка при открытии файла журнала\n");
    exit(1);
}

// Функция записи ответа в журнал
int printResult(const char* content) {
    FILE* f1 = fopen(logName, "a");
    if (f1 != NULL) {
        time_t rawtime;
        fprintf(f1, "%s %s", ctime(&rawtime), content);
        fclose(f1);
        return 0;
    } else {
        return 1;
    }
}

// Функция диагностики записи ответа в журнал
void diagPrint() {
    printf("Ошибка при записи в файл\n");
    exit(1);
}

// Функция диагностики создания файла журнала (ошибка при создании)
void diagCreateLog() {
    printf("Ошибка при создании файла журнала\n");
    exit(1);
}

// Функция диагностики проверки файла журнала (ошибка при поиске файла)
void diagCheckLog() {
    printf("Ошибка при проверке файла журнала\n");
    exit(1);
}
