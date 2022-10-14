#include "gpaint_filters.h"
using namespace Filters;

void Gray::transform(Image &image) {
    size_t width  = image.GetWidth();
    size_t height = image.GetHeight();
    RGBColor** pixels = image.GetPixelArray();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            int midpix = 
            pixels[y][x].R + 
            pixels[y][x].G + 
            pixels[y][x].B;

            midpix /= 3;
            pixels[y][x] = RGBColor(midpix, midpix, midpix);
        }
    }
}
