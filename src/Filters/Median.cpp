#include "gpaint_filters.h"
#include <vector>
#include <algorithm>
using namespace Filters;

void Median::transform(Image &image) {
    int width  = image.GetWidth();
    int height = image.GetHeight();
    RGBColor** pixels = image.GetPixelArray();

    Image new_img(width, height);
    RGBColor** new_pixels = new_img.GetPixelArray();
    printf("ok\n");

    std::vector<RGBColor> colors;
    colors.reserve(((2 * _shift + 1) * (2 * _shift + 1), RGB::Null));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x)
        {
            for (int dy = y - _shift; dy <= y + _shift; ++dy) {
                for (int dx = x - _shift; dx <= x + _shift; ++dx) {
                    if (0 <= dx && dx < width && 0 <= dy && dy < height) {
                        colors.push_back(pixels[dy][dx]);
                    }
                }
            }
            std::sort(colors.begin(), colors.end(), brightnessCmp);
            new_pixels[y][x] = colors[colors.size() / 2];
            colors.clear();
        }
    }
    image = new_img;
}

int Median::brightnessCmp(RGBColor first, RGBColor second) {
    int abs1 = (int)first.R * first.R + (int)first.G * first.G + (int)first.B * first.B;
    int abs2 = (int)second.R * second.R + (int)second.G * second.G + (int)second.B * second.B;
    return abs1 < abs2;
}