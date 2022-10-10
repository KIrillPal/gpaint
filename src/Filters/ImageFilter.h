#pragma once
#include "../Image.h"

class ImageFilter {
public:
    virtual void transform(Image &image) = 0;
    virtual ~ImageFilter() = default;
protected:
private:
};