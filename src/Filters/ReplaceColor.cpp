#include "gpaint_filters.h"
using namespace Filters;

ReplaceColor::ReplaceColor() : ReplaceColor(RGB::Null, RGB::Null) {}

ReplaceColor::ReplaceColor(RGBColor from, RGBColor to) : _from_color(from), _to_color(to) {}

void ReplaceColor::transform(Image &image) {
    size_t width  = image.GetWidth();
    size_t height = image.GetHeight();
    RGBColor** pixels = image.GetPixelArray();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            if (pixels[y][x] == _from_color) {
                pixels[y][x] = _to_color;
            }
        }
    }
}
