#include <algorithm>
#include <cmath>
#include "gpaint_filters.h"
using namespace Filters;

Convolution::Convolution() : _conv_matrix(nullptr), _conv_width(0), _conv_height(0) {}

void Convolution::init(const float* CONV_MATRIX, int CONV_WIDTH, int CONV_HEIGHT) {
    _conv_matrix = CONV_MATRIX;
    _conv_width  = CONV_WIDTH;
    _conv_height = CONV_HEIGHT;
}

void Convolution::transform(Image &image) {
    if (_conv_matrix == nullptr)
        return;

    size_t     width  = image.GetWidth();
    size_t     height = image.GetHeight();
    RGBColor** pixels = image.GetPixelArray();

    Image new_img(width, height);
    RGBColor** new_pixels = new_img.GetPixelArray();

    size_t bound_x = _conv_width / 2;
    size_t bound_y = _conv_height / 2;
    for (size_t y = bound_y; bound_y + y < height; ++y) {
        for (size_t x = bound_x; bound_x + x < width; ++x) {
            makeConvolution(pixels, new_pixels[y][x], x, y);
        }
    }
    completeBounds(new_pixels, pixels, width, height);
    image = new_img;
}

inline void Convolution::makeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t x, size_t y) {
    float R = 0, G = 0, B = 0;
    const float* _conv_item = _conv_matrix;
    int shift_x = _conv_width / 2;
    int shift_y = _conv_height / 2;

    for (int ox = x-shift_x; ox <= x+shift_x; ++ox) {
        for (int oy = y-shift_y; oy <= y+shift_y; ++oy) {
            float coefficient = *(_conv_item++);
            R += src_pixels[oy][ox].R * coefficient;
            G += src_pixels[oy][ox].G * coefficient;
            B += src_pixels[oy][ox].B * coefficient;
        }
    }

    if (R > 255)    R = 255;
    else if (R < 0) R = 0;
    if (G > 255)    G = 255;
    else if (G < 0) G = 0;
    if (B > 255)    B = 255;
    else if (B < 0) B = 0;
    dst_pixel = RGBColor(
            (uint8_t)std::roundf(R),
            (uint8_t)std::roundf(G),
            (uint8_t)std::roundf(B)
            );
}

inline void Convolution::makeSafeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t width, size_t height, int x, int y) {
    float R = 0, G = 0, B = 0;
    const float* _conv_item = _conv_matrix;
    int shift_x = _conv_width / 2;
    int shift_y = _conv_height / 2;

    float added_area = 0;
    for (int ox = x-shift_x; ox <= x+shift_x; ++ox) {
        for (int oy = y-shift_y; oy <= y+shift_y; ++oy) {
            float coefficient = *(_conv_item++);
            // first difference between safe and not
            if (0 <= ox && ox < width && 0 <= oy && oy < height) {
                R += src_pixels[oy][ox].R * coefficient;
                G += src_pixels[oy][ox].G * coefficient;
                B += src_pixels[oy][ox].B * coefficient;
                added_area += coefficient;
            }
        }
    }

    //second difference between safe and not
    float bright_coefficient = 1.f / added_area;
    R *= bright_coefficient;
    G *= bright_coefficient;
    B *= bright_coefficient;

    if (R > 255)    R = 255;
    else if (R < 0) R = 0;
    if (G > 255)    G = 255;
    else if (G < 0) G = 0;
    if (B > 255)    B = 255;
    else if (B < 0) B = 0;
    dst_pixel = RGBColor(
            (uint8_t)std::roundf(R),
            (uint8_t)std::roundf(G),
            (uint8_t)std::roundf(B)
            );
}

void Convolution::completeBounds(RGBColor** dst_pixels, RGBColor **src_pixels, size_t width, size_t height) {
    size_t bound_x = _conv_width / 2;
    size_t bound_y = _conv_height / 2;
    bound_x = std::min(bound_x,  width / 2 + 1);
    bound_y = std::min(bound_y, height / 2 + 1);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < bound_x; ++x) {
            makeSafeConvolution(src_pixels, dst_pixels[y][x], width, height, x, y);
            makeSafeConvolution(src_pixels, dst_pixels[y][width-x-1], width, height, width-x-1, y);
        }
    }

    for (int x = bound_x; x < width - bound_x; ++x) {
        for (int y = 0; y < bound_y; ++y) {
            makeSafeConvolution(src_pixels, dst_pixels[y][x], width, height, x, y);
            makeSafeConvolution(src_pixels, dst_pixels[height-y-1][x], width, height, x, height-y-1);
        }
    }
}