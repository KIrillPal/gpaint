#pragma once
#include "color.h"

class Image {
public:
    Image();
    Image(size_t width, size_t height);
    Image(size_t width, size_t height, RGBColor fill_color);
    Image(const Image& other);

    size_t     GetWidth()      const;
    size_t     GetHeight()     const;
    RGBColor** GetPixelArray() const;

    RGBColor GetPixel(size_t row, size_t column) const;
    void     SetPixel(size_t row, size_t column, RGBColor color);

    Image& operator = (const Image& other);
    ~Image();
protected:
private:
    RGBColor** _data;
    size_t _width, _height;

    void reallocData(size_t width, size_t height);
    void copyData(RGBColor** source);
    void fillData   (RGBColor fill_color);
};