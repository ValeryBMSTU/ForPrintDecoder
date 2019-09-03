/*
    PNGdecode
    Vitaly Shulman
    Никакие права не защищены
*/

#include "lodepng.h" // Левая библиотека, которую я использую для декодирования png
#include <iostream>
#include "stdlib.h"
#include <string>
#include <fstream>
#include <dirent.h>
#include "windows.h"
#include <vector>

#define MAX_COL_LOGS 8

//Decode from disk to raw pixels with a single function call
void decodeOneStep(const char *, const char *);

/* Функция для удаления из строки подстроки. Взята с просторов интернета */
char* f_(char *, char *);

/* Функция для проверки существования папок и создания, если их не было */
int dir_exist_check(const char *, bool &, std::string &);

/* Функция для формирования списка файлов в каталоге, удовлетворящих заданному формату (png, h, и т.д.) */
int dir_image_directory(const char *, const char *, std::vector <std::string> &);

/* Функция, выполняющая преобразование файлов png в h */
int convert_png_to_h(int, std::vector <std::string> &);

int main(int argc, char *argv[]) {

    bool error_flag = false; //Флаг для ошибок
    std::string error_message = "Unknown error";
    char png_format[] = ".png\0";
    char  h_format[] = ".h\0";

    if (!error_flag)
        dir_exist_check("PNGfolder", error_flag, error_message);
    if (!error_flag)
        dir_exist_check("Hfolder", error_flag, error_message);

    int png_count = 0, h_count = 0;
    std::vector <std::string> png_list;
    std::vector <std::string> h_list;

    if (!error_flag)
    {
        png_count = dir_image_directory("PNGfolder", png_format, png_list);
    
        convert_png_to_h(png_count, png_list);

        h_count = dir_image_directory("Hfolder", h_format, h_list);
    }

    system("pause");
}


void decodeOneStep(const char *png_filename, const char *h_filename) {
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;
    const unsigned char black = 0;
    const unsigned char white = 0;


    //decode
    unsigned error = lodepng::decode(image, width, height, png_filename); //Сложная функция для декодирования png файла. Взята с просторов интернета. В подробности работы вникать не рекомендуется.

    //if there's an error, display it
    if (error)
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

    int curent_width = 0;
    int curent_hight = 0;
    FILE * ptrFile = fopen(h_filename, "w");
 
    if (ptrFile != NULL)
    {
        std::cout << "Convert process " << png_filename << " --> " << h_filename << ": "; 

        /* Формируем первую фоловину файла h */
        std::string head_str= "#ifndef IMAGE_H_\n#define IMAGE_H_\n\n#define COLS ";
        head_str = head_str + std::to_string(height);
        head_str = head_str + "\n\nstatic const byte IMAGE[COLS][";
        head_str = head_str + std::to_string(width);
        head_str = head_str + "] PROGMEM = {\n{";

        fputs(head_str.c_str(), ptrFile); 

        /* Формируем матрицу пикселей */
        for (std::vector<unsigned char>::iterator it = image.begin(); it != image.end(); it = it + 4)
        {
            curent_width++;
            if (*it == black)
            {
                fputs("0b00000000", ptrFile); //Черный пиксель
            }
            else
            {
                fputs("0b11111111", ptrFile); //Белый пиксель
            }
            
            if (curent_width == width) //Условия для перехода к формированию следующей строки пикселей
            {
                curent_width = 0;
                curent_hight++;
                if (curent_hight != height)
                {
                    fputs("},\n{", ptrFile);
                }
                else
                {
                    fputs("}", ptrFile);
                }
            }
            else
            {
                if (curent_width != width)
                    fputs(",", ptrFile);
            }
        }
        fputs("\n};\n\n#endif\n", ptrFile); //Добавляем служебную информацию в конец файла
        fclose (ptrFile); //Заканчиваем формирование файла и закрываем его
        std::cout << "SUCCESS." << std::endl;
    }
}

/* Функция для удаления из строки подстроки. Взята с просторов интернета */
char* f_(char *src, char *src_) {
    char *t;

    do
    {
        t = strstr(src, src_);
        if (t != NULL)
        {
            char *t_ = t + strlen(src_);
            strcpy(t, t_);
        }
        else
            break;
    } while (true);
    return src;
}

/* Функция для проверки существования папок и создания, если их не было */
int dir_exist_check(const char *folder_name, bool &error_flag, std::string &error_message) {
    DIR *dir = opendir(folder_name); //Пытаемся открыть директорию с исходными картинками
    if (dir)                         //Проверка существования директории
    {
        /* Directory exists. */
        std::cout << "Directory '" << folder_name << "' exsists." << std::endl;
        closedir(dir); //Закрытие директории (иначе утечка памяти)
    }
    else
    {
        /* Directory dose not exist */
        std::cout << "Directory '" << folder_name << "' does not exist" << std::endl;
        CreateDirectory(folder_name, NULL); //Создание новой директории
        dir = opendir(folder_name);         //Пытаемся открыть созданную директорию
        if (dir)                            //Проверка существования новой директории
        {
            /* Directory has created */
            std::cout << "Directory '" << folder_name << "' has created" << std::endl;
            closedir(dir); //Закрытие директории (иначе утечка памяти)
        }
        else
        {
            /* Directory has not created */
            std::cout << "Directory '" << folder_name << "' did not creat" << std::endl;
            std::string fail_folder_name = folder_name; 
            error_message = "Critical error: try to create directory failed" + fail_folder_name;
            error_flag = true; //Устанавливаем флаг ошибки
        }
    }
    return 0;
}

/* Функция для формирования списка файлов в каталоге, удовлетворящих заданному формату (png, h, и т.д.) */
int dir_image_directory(const char *folder_name, const char *format, std::vector <std::string> &list)
{
    /* Формируем путь до необходимой папки для получения списка файлов в ней */
    TCHAR buffer[MAX_PATH];
    GetCurrentDirectory(sizeof(buffer), buffer);
    strcat(buffer, "\\");
    strcat(buffer, folder_name);
    strcat(buffer, "\\*");

    std::cout << "Search directory for " << format << ":  " << buffer << std::endl;

    /* Производим настройки и объявляем переменные для ОС Windows */
    setlocale(LC_ALL, "");
    HANDLE search_location;
    WIN32_FIND_DATA founded_file;
    int curent_format_count = 0;
    
    std::cout << "The list of files for " << folder_name << " directory:" << std::endl;

    /* Производим поиск файлов формата, записанного в переменную format */
    search_location = FindFirstFile(buffer, &founded_file);
    int current_col = 0; //Счетчик колонок для более красивого вывода логов
    while (FindNextFile(search_location, &founded_file)) // != NULL
    {
        std::cout << founded_file.cFileName << "\t";

        current_col++;
        if (current_col == MAX_COL_LOGS) { std::cout << "\n"; current_col = 0; }

        std::string file_name = founded_file.cFileName;
        int pos = -1;
        if( (pos = file_name.find(format) ) != -1 ) //Если файл соответсвуте формату - добавляем его в список найденных файлов list
        {
            curent_format_count++;
            list.push_back(file_name);
        }
    }
    std::cout << "\n\n" << "Total count of " << format << " files: " << curent_format_count << "\n";
    return curent_format_count;
}

/* Функция, выполняющая преобразование файлов png в h */
int convert_png_to_h(int png_count, std::vector <std::string> &list)
{
    std::cout << std::endl;
    for (int i = 0; i < png_count; i++)
    {
        /* Формируем имя для будущего файла h */
        char *str = new char[32];
        strcpy(str, list[i].c_str());
        char *str_cmp = ".png";
        f_(str, str_cmp);
        strcat(str, ".h"); 

        /* Присваиваем переменным именя для последущей передачи в функцию */
        std::string png_filename = "PNGfolder\\" + list[i];
        std::string h_filename = "Hfolder\\" + std::string(str);

        decodeOneStep(png_filename.c_str(), h_filename.c_str()); //Декодирует png-рисунок и создает h-файл с таким же названием
    }
    std::cout << std::endl;
    return 0;
}