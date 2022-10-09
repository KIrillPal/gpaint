#include <iostream>
#include "Image.h"
#include "BMP.h"

int main() {
    try {
        Image img;
        BMPReader::loadFromFile("img.bmp",img);
        BMPReader::saveToFile("kek.bmp", img);
    } catch (const std::exception& ex) {
        printf("Got exception: %s\n", ex.what());
    }
    return 0;
}