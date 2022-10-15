#include "gpaint_filters.h"
#include "../gpaint_exception.h"
using namespace Filters;

Crop::Crop(size_t size_x, size_t size_y, size_t pos_x, size_t pos_y) {
    _size_x = size_x; 
    _size_y = size_y; 
    _pos_x = pos_x; 
    _pos_y = pos_y;
}

void Crop::transform(Image &image) {
    size_t width  = image.GetWidth();
    size_t height = image.GetHeight();
    RGBColor** pixels = image.GetPixelArray();

    if (_pos_x + _size_x > width || _pos_y + _size_y > height) {
        GPAINT_EXCEPTION("Chosen crop area (%zu %zu, %zu %zu) is out of image bounds",
                         _pos_x, _pos_y, _size_x, _size_y);
    }

    Image new_img(_size_x, _size_y);
    RGBColor** new_pixels = new_img.GetPixelArray();

    for (size_t y = 0; y < _size_y; ++y) {
        for (size_t x = 0; x < _size_x; ++x) {
            size_t y_pos = height - _pos_y - _size_y + y;
            new_pixels[y][x] = pixels[y_pos][_pos_x + x];
        }
    }
    image = new_img;
}
