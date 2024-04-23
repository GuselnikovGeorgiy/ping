## 
## ДЕКЛАРАЦИЯ БИБЛИОТЕК
##
import zipfile                                          # Работа с архивами ZIP
import os                                               # Взаимодействие с операционной системой
from datetime import datetime                           # Работа с датами и временем
import sys

## 
## ДЕКЛАРАЦИЯ И ИНИЦИАЛИЗАЦИЯ ГЛОБАЛЬНЫХ ПЕРЕМЕННЫХ
##
data = datetime().now                                   # Глобальная переменная для хранения даты
data_str = data.strftime("%Y-%m-%d_%H:%M:%S")           # Преобразование даты в строчный формат

##
## ДЕКЛАРАЦИЯ ФУНКЦИЙ
##
def zip(file_name, archive_name):                       # Функция для архивации файла
    with zipfile.ZipFile(archive_name, 'w') as zipf:    # Открываем архив для записи файла            
        zipf.write(file_name, arcname=file_name)        # Добавляем файл в архив
    os.remove(file_name)                                # Удаляем оригинал файла

def main():                                             # Главная функция
    # Получение пути
    file_name = sys.argv[1]
    file_name = file_name.replace(" ", "")

    index = file_name.rfind('/')
    archive_name = file_name[:index]
    archive_name = archive_name.replace("txt","") 

    archive_name = "/etc/log_zip" + archive_name + "_" + data_str

    #Начало работы
    zip(file_name, archive_name)

main()