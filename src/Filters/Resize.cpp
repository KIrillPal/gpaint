#include "gpaint_filters.h"
#include <cmath>
#include "../gpaint_exception.h"
using namespace Filters;

Resize::Resize(size_t width, size_t height) : _width(width), _height(height) {}

void Resize::transform(Image &image) {
    size_t width  = image.GetWidth();
    size_t height = image.GetHeight();
    RGBColor** pixels = image.GetPixelArray();

    if (_width == 0 || _height == 0) {
        GPAINT_EXCEPTION("New size must be positive");
    }

    Image new_img(_width, height);
    RGBColor** new_pixels = new_img.GetPixelArray();

    for (size_t y = 0; y < height; ++y) {
        double xl = 0;
        double xr = 0;
        for (size_t x = 0; x < _width; ++x) {
            xl = xr;
            xr = (x + 1) * width * 1.0 / _width;
            new_pixels[y][x] = convLineX(pixels, y, xl, xr);
        }
    }

    image = Image(_width, _height);
    pixels = image.GetPixelArray();

    for (size_t x = 0; x < _width; ++x) {
        double yl = 0;
        double yr = 0;
        for (size_t y = 0; y < _height; ++y) {
            yl = yr;
            yr = (y + 1) * height * 1.0 / _height;
            pixels[y][x] = convLineY(new_pixels, x, yl, yr);
        }
    }
}


RGBColor Resize::convLineX(RGBColor** data, int y, double x1, double x2) {
    const double EPS = 0.000001;

    int x1_bound = std::ceil(x1)+EPS;
    int x2_bound = std::floor(x2)+EPS;
    if (x1_bound > x2_bound) {
        return data[y][x2_bound];
    }
    double R = 0, G = 0, B = 0;

    double t = x1_bound - x1;
    if (t >= EPS) {
        R += data[y][x1_bound - 1].R * t;
        G += data[y][x1_bound - 1].G * t;
        B += data[y][x1_bound - 1].B * t;
    }

    t = x2 - x2_bound;
    if (t >= EPS) {
        R += data[y][x2_bound].R * t;
        G += data[y][x2_bound].G * t;
        B += data[y][x2_bound].B * t;
    }

    for (int i = x1_bound; i < x2_bound; ++i) {
        R += data[y][i].R;
        G += data[y][i].G;
        B += data[y][i].B;
    }

    double weight = x2-x1;
    return RGBColor(std::roundf(R / weight),
                    std::roundf(G / weight),
                    std::roundf(B / weight)
                    );
}

RGBColor Resize::convLineY(RGBColor** data, int x, double y1, double y2) {
    const double EPS = 0.000001;

    int y1_bound = std::ceil(y1)+EPS;
    int y2_bound = std::floor(y2)+EPS;
    if (y1_bound > y2_bound) {
        return data[y2_bound][x];
    }
    double R = 0, G = 0, B = 0;

    double t = y1_bound - y1;
    if (t >= EPS) {
        R += data[y1_bound - 1][x].R * t;
        G += data[y1_bound - 1][x].G * t;
        B += data[y1_bound - 1][x].B * t;
    }

    t = y2 - y2_bound;
    if (t >= EPS) {
        R += data[y2_bound][x].R * t;
        G += data[y2_bound][x].G * t;
        B += data[y2_bound][x].B * t;
    }

    for (int i = y1_bound; i < y2_bound; ++i) {
        R += data[i][x].R;
        G += data[i][x].G;
        B += data[i][x].B;
    }

    double weight = y2-y1;
    return RGBColor(std::roundf(R / weight),
                    std::roundf(G / weight),
                    std::roundf(B / weight)
    );
}