#include <iostream>
#include "Image.h"
#include "BMP.h"
#include "Filters/gpaint_filters.h"

int main() {
    ImageFilter* gauss = new Filters::Gauss(71, 11);

    try {
        Image img;
        BMPReader::loadFromFile("images/img.bmp",img);

        gauss->transform(img);

        BMPReader::saveToFile("images/kek.bmp", img);
    } catch (const std::exception& ex) {
        printf("Got exception: %s\n", ex.what());
    }

    delete gauss;
    return 0;
}