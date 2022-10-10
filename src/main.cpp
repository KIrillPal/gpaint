#include <iostream>
#include "Image.h"
#include "BMP.h"
#include "Filters/gpaint_filters.h"

int main() {
    ImageFilter* neg = new Filters::Negative();
    ImageFilter* rep = new Filters::ReplaceColor(RGB::Black, RGB::White);

    try {
        Image img;
        BMPReader::loadFromFile("images/snail.bmp",img);

        neg->transform(img);
        rep->transform(img);

        BMPReader::saveToFile("images/kek.bmp", img);
    } catch (const std::exception& ex) {
        printf("Got exception: %s\n", ex.what());
    }

    delete neg;
    delete rep;
    return 0;
}