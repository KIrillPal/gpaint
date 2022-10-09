#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "color.h"
#include "Image.h"

Image::Image() : _data(nullptr), _width(0), _height(0) {}

Image::Image(size_t width, size_t height) : _width(width), _height(height), _data(nullptr) {
    reallocData(width, height);
}

Image::Image(size_t width, size_t height, RGBColor fill_color) : Image(width, height) {
    fillData(fill_color);
}

Image::Image(const Image& other) : Image(other._width, other._height) {
    size_t elems = _width*_height;
    copyData(other._data);
}


size_t Image::GetWidth() const {
    return _width;
}
size_t Image::GetHeight() const {
    return _height;
}

RGBColor** Image::GetPixelArray() const {
    return _data;
}

RGBColor Image::GetPixel(size_t row, size_t column) const
{
    if (row >= _height || column >= _width) {
        char errorMessage[64];
        snprintf(errorMessage, 64, "Failed to get point (%zu, %zu) with size (%zu, %zu)", 
            column, row,
            _width, _height
            );
        throw std::out_of_range(errorMessage);
    }
    return _data[row][column];
}
void Image::SetPixel(size_t row, size_t column, RGBColor color) 
{
    if (row >= _height || column >= _width) {
        char errorMessage[64];
        snprintf(errorMessage, 64, "Failed to get point (%zu, %zu) with size (%zu, %zu)", 
            column, row,
            _width, _height
            );
        throw std::out_of_range(errorMessage);
    }
    _data[row][column] = color;
}

Image::~Image() {
    free(_data);
}

void Image::reallocData(size_t width, size_t height) 
{
    size_t header_size = height * sizeof(RGBColor*);
    size_t row_size    = width  * sizeof(RGBColor);
    size_t data_size   = height * row_size;
    void* new_data = realloc(_data, header_size + data_size);

    if (new_data == nullptr) {
        free(_data);
        char errorMessage[64];
        snprintf(errorMessage, 64, "Failed to allocate %d bytes", header_size + data_size);
        throw std::runtime_error(errorMessage);
    }

    _data = reinterpret_cast<RGBColor**>(new_data);

    uint8_t* new_data_shift = reinterpret_cast<uint8_t*>(new_data);
    for (int i = 0; i < height; ++i) {
        _data[i] = reinterpret_cast<RGBColor*>(new_data_shift + header_size + i * row_size);
    }
}

void Image::fillData(RGBColor fill_color) {
    for (size_t y = 0; y < _height; ++y) {
        for (size_t x = 0; x < _width; ++x) {
            _data[y][x] = RGB::Blue;
        }
    }

}

void Image::copyData(RGBColor **source) {
    size_t npixels = _width * _height;
    RGBColor* raw_src = source[0];
    RGBColor* raw_dst = _data [0];

    if (std::copy(raw_src, raw_src + npixels, raw_dst) != raw_dst + npixels) {
        throw std::runtime_error("Failed to copy Image data.");
    }
}

Image& Image::operator=(const Image &other) {
    if (this == std::addressof(other)) {
        return *this;
    }
    if (_width != other._width || _height != other._height) {
        _width  = other._width;
        _height = other._height;
        reallocData(_width, _height);
    }
    copyData(other._data);

    return *this;
}