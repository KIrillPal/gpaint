#include <cmath>
#include "gpaint_filters.h"
using namespace Filters;

Gauss::Gauss(float dispersion, size_t filter_size) {
    if (filter_size == 0)
        filter_size = std::ceil(6 * dispersion);
    _kernel_size = filter_size;
    _kernel      = makeKernel(filter_size, dispersion);
}

Gauss::~Gauss() {
    delete[] _kernel;
}

float* Gauss::makeKernel(size_t filter_size, float dispersion) {
    float* kernel = new float[filter_size];
    int shift = filter_size / 2;
    float total_sum = 0;

    for (int i = 0; i < filter_size; ++i) {
        float value = gaussFunction(i-shift, dispersion);
        kernel[i] = value;
        total_sum += value;
    }

    for (size_t i = 0; i < filter_size; ++i) {
        kernel[i] /= total_sum;
    }
    return kernel;
}

float Gauss::gaussFunction(float x, float y, float sigma) {
    return std::exp(-(x*x+y*y)/(2*sigma*sigma)) / (2*M_PI*sigma*sigma);
}

float Gauss::gaussFunction(float x, float sigma) {
    return std::exp(-(x*x)/(2*sigma*sigma)) / (std::sqrt(2*M_PI)*sigma);
}

void Gauss::transform(Image &image) {
    init(_kernel, _kernel_size, 1);
    Convolution::transform(image);
    init(_kernel, 1, _kernel_size);
    Convolution::transform(image);
}
