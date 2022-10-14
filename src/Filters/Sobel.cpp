#include <cmath>
#include "gpaint_filters.h"
using namespace Filters;

Sobel::Sobel() {
    init(CONV_Gx[0], CONV_SIZE, CONV_SIZE);
}

inline void Sobel::makeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t x, size_t y) {
    int shift = CONV_SIZE / 2;
    float R = 0, G = 0, B = 0;
    float gx_value = 0, gy_value = 0;

    for (int ox = x-shift; ox <= x+shift; ++ox) {
        for (int oy = y-shift; oy <= y+shift; ++oy)
        {
            float bright = getPixelBrightness(src_pixels[oy][ox]);
            gx_value += bright * CONV_Gx[oy-y+shift][ox-x+shift];
            gy_value += bright * CONV_Gy[oy-y+shift][ox-x+shift];
        }
    }

    float final = std::sqrt(gx_value*gx_value + gy_value*gy_value);
    if (final > 255) final = 255;
    else if (final < 0) final = 0;

    uint8_t channel = std::roundf(final);
    dst_pixel = RGBColor(channel, channel, channel);
}

float Sobel::getPixelBrightness(RGBColor pixel) {
    return (pixel.R + pixel.G + pixel.B) / 3.0;
}

inline void Sobel::makeSafeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t width, size_t height, int x, int y) {
    int shift = CONV_SIZE / 2;
    float R = 0, G = 0, B = 0;

    float gx_value = 0, gy_value = 0;
    float added_area = 0;

    for (int ox = x-shift; ox <= x+shift; ++ox) {
        for (int oy = y-shift; oy <= y+shift; ++oy)
        {
            if (0 <= ox && ox < width && 0 <= oy && oy < height) {
                float bright = getPixelBrightness(src_pixels[oy][ox]);
                gx_value += bright * CONV_Gx[oy - y + shift][ox - x + shift];
                gy_value += bright * CONV_Gy[oy - y + shift][ox - x + shift];
                added_area += CONV_Gx[oy - y + shift][ox - x + shift];
                added_area += CONV_Gy[oy - y + shift][ox - x + shift];
            }
        }
    }

    float bright_coefficient = 2.f / added_area;

    float final = std::sqrt(gx_value*gx_value + gy_value*gy_value) * bright_coefficient;
    if (final > 255) final = 255;
    else if (final < 0) final = 0;

    uint8_t channel = std::roundf(final);
    dst_pixel = RGBColor(channel, channel, channel);
}
