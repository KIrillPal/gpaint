#pragma once
#include "ImageFilter.h"

namespace Filters {
    class Negative;
    class ReplaceColor;
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