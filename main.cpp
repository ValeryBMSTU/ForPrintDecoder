/*
LodePNG Examples

Copyright (c) 2005-2012 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include "lodepng.h"
#include <iostream>
#include "stdlib.h"
#include <string>
#include <fstream>
#include <cstdint>
#include <io.h>
#include <dirent.h>
#include <errno.h>
#include "windows.h"
// #include <experimental/filesystem> // C++-standard header file name
// #include <filesystem> // Microsoft-specific implementation header file name
// using namespace std::experimental::filesystem::v1;
/*
3 ways to decode a PNG from a file to RGBA pixel data (and 2 in-memory ways).
*/

//g++ lodepng.cpp example_decode.cpp -ansi -pedantic -Wall -Wextra -O3

//Example 1
//Decode from disk to raw pixels with a single function call
void decodeOneStep(const char *filename)
{
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //decode
    unsigned error = lodepng::decode(image, width, height, filename);

    //if there's an error, display it
    if (error)
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

    int curent_width = 0;

    for (std::vector<unsigned char>::iterator it = image.begin(); it != image.end(); it = it + 4)
    {
        if (curent_width == width)
        {
            std::cout << std::endl;
            curent_width = 0;
        }
        else
        {
            curent_width++;
        }

        std::cout << *it;
    }
}

//Example 2
//Load PNG file from disk to memory first, then decode to raw pixels in memory.
void decodeTwoSteps(const char *filename)
{
    std::vector<unsigned char> png;
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //load and decode
    unsigned error = lodepng::load_file(png, filename);
    if (!error)
        error = lodepng::decode(image, width, height, png);

    //if there's an error, display it
    if (error)
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
}

//Example 3
//Load PNG file from disk using a State, normally needed for more advanced usage.
void decodeWithState(const char *filename)
{
    std::vector<unsigned char> png;
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;
    lodepng::State state; //optionally customize this one

    unsigned error = lodepng::load_file(png, filename); //load the image file with given filename
    if (!error)
        error = lodepng::decode(image, width, height, state, png);

    //if there's an error, display it
    if (error)
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
    //State state contains extra information about the PNG such as text chunks, ...
}

int main(int argc, char *argv[])
{
    FILE *file = NULL; //Переменная для хранения названия файла
    DIR *dir = NULL; //Переменная для хранения названия директории
    bool error_flag = false; //Флаг для ошибок
    std::string error_message = "Неизвестная ошибка";

    dir = opendir("PNGfolder"); //Пытаемся открыть директорию с исходными картинками
    if (dir) //Проверка существования директории
    {
        /* Directory exists. */
        std::cout << "Директория 'PNGfolder' существует." << std::endl;
        closedir(dir); //Закрытие директории (иначе утечка памяти)
    }
    else 
    {
        /* Directory dose not exist */
        std::cout << "Директория 'PNGfolder' не существует" << std::endl;
        CreateDirectory( "PNGfolder", NULL ); //Создание новой директории
        dir = opendir("PNGfolder"); //Пытаемся открыть созданную директорию
        if (dir) //Проверка существования новой директории
        {
            /* Directory has created */
            std::cout << "Директория 'PNGfolder' успешно создана" << std::endl;
            closedir(dir); //Закрытие директории (иначе утечка памяти)
        }
        else 
        {
            /* Directory has not created */
            std::cout << "Не удалость создать директорию 'PNGfolder'" << std::endl;
            error_flag = true; //Устанавливаем флаг ошибки
        }
    }

    if (error_flag)
    {
        std::cout << error_message << std::endl;
        return -1;
    }

    const char *filename = argc > 1 ? argv[1] : "test.png";
    decodeOneStep(filename);
    system("pause");
}
