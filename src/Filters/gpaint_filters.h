#pragma once
#include "ImageFilter.h"
#include <cstdio>

namespace Filters {
    class Negative;
    class ReplaceColor;
    class Clarify;
    class Convolution;
    class Gauss;
    class Gray;
    class Edges;
    class Sobel;
    class Median;
    class Crop;
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
    void init(const float* CONV_MATRIX, int CONV_WIDTH, int CONV_HEIGHT);
    ~Convolution()               override = default;
    void transform(Image &image) override;
private:
    const float* _conv_matrix;
    int  _conv_width;
    int  _conv_height;
    virtual void makeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t x, size_t y);
    void completeBounds (RGBColor** dst_pixels, RGBColor** src_pixels, size_t width, size_t height);
    virtual void makeSafeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t width, size_t height, int x, int y);
};

class Filters::Clarify : public Convolution {
public:
    Clarify() {init(CONV_MATRIX[0], CONV_SIZE, CONV_SIZE);};
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
    Gauss(float dispersion, size_t filter_size = 0);
    ~Gauss() override;

    void transform(Image& image) override;
private:
    size_t _kernel_size;
    float* _kernel;
    float* makeKernel(size_t filter_size, float dispersion);
    float gaussFunction(float x, float y, float sigma);
    float gaussFunction(float x, float sigma);
};

class Filters::Gray : public ImageFilter {
public:
    Gray() = default;
    ~Gray()                      override = default;
    void transform(Image &image) override;
private:
};

class Filters::Edges : public Convolution {
public:
    Edges() {init(CONV_MATRIX[0], CONV_SIZE, CONV_SIZE);};
private:
    const int   CONV_SIZE = 3;
    const float CONV_MATRIX[3][3] = {
            {1, 2, 1},
            {0,  0, 0},
            {-1, -2, -1}
    };
};

class Filters::Sobel : public Convolution {
public:
    Sobel();
private:
    void makeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t x, size_t y) override;
    void makeSafeConvolution(RGBColor** src_pixels, RGBColor& dst_pixel, size_t width, size_t height, int x, int y) override;
    float getPixelBrightness(RGBColor pixel);

    const int   CONV_SIZE = 3;
    const float CONV_Gx[3][3] = {
            {-1, -2, -1},
            {0,  0, 0},
            {1, 2, 1}
    };
    const float CONV_Gy[3][3] = {
            {-1, 0, 1},
            {-2,  0, 2},
            {-1, 0, 1}
    };
};

class Filters::Median : public ImageFilter {
public:
    Median() = default;
    ~Median()                    override = default;
    void transform(Image &image) override;
private:
    static int brightnessCmp(RGBColor first, RGBColor second);
    int _shift = 2;
};

class Filters::Crop : public ImageFilter {
public:
    Crop() : Crop(0, 0, 0, 0) {};
    Crop(size_t size_x, size_t size_y, size_t pos_x, size_t pos_y);
    ~Crop() override = default;
    void transform(Image &image) override;
private:
    size_t _size_x, _size_y, _pos_x, _pos_y;
};