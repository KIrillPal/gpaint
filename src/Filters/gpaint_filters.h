#pragma once
#include "ImageFilter.h"
#include <cstdio>

namespace Filters {
    class Negative;
    class ReplaceColor;
    class Clarify;
    class Convolution;
    class Gauss;
}

class Filters::Negative : public ImageFilter {
public:
    Negative()  = default;
    ~Negative()                  override = default;
    void transform(Image &image) override;
private:
};

class Filters::ReplaceColor : public ImageFilter {
public:
    ReplaceColor();
    ReplaceColor(RGBColor from, RGBColor to);
    ~ReplaceColor()              override = default;
    void transform(Image &image) override;
private:
    RGBColor _from_color;
    RGBColor _to_color;
};

class Filters::Convolution : public ImageFilter {
public:
    Convolution();
    void init(const float* CONV_MATRIX, int CONV_SIZE);
    ~Convolution()               override = default;
    void transform(Image &image) override;
private:
    const float* _conv_matrix;
    int  _conv_size;
    void makeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t x, size_t y);
    void completeBounds (RGBColor** dst_pixels, RGBColor** src_pixels, size_t width, size_t height);
    void makeSafeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t width, size_t height, int x, int y);
};

class Filters::Clarify : public Convolution {
public:
    Clarify() {init(CONV_MATRIX[0], CONV_SIZE);};
private:
    const int   CONV_SIZE = 3;
    const float CONV_MATRIX[3][3] = {
            {-1, -1, -1},
            {-1,  9, -1},
            {-1, -1, -1}
    };
};

class Filters::Gauss : public Convolution {
public:
    Gauss(size_t filter_size, float dispersion);
    ~Gauss() override;
private:
    size_t _kernel_size;
    float* _kernel;
    float* makeKernel(size_t filter_size, float dispersion);
    float gaussFunction(float x, float y, float sigma);
};