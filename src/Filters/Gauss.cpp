#include <cmath>
#include "gpaint_filters.h"
using namespace Filters;

Gauss::Gauss(size_t filter_size, float dispersion) {
    _kernel_size = filter_size;
    _kernel      = makeKernel(filter_size, dispersion);
    init(_kernel, _kernel_size);
}

Gauss::~Gauss() {
    delete[] _kernel;
}

float* Gauss::makeKernel(size_t filter_size, float dispersion) {
    float* kernel = new float[filter_size * filter_size];
    int shift = filter_size / 2;
    float total_sum = 0;

    for (int i = 0; i < filter_size; ++i) {
        for (int j = 0; j < filter_size; ++j)
        {
            float value = gaussFunction(i-shift, j-shift, dispersion);
            kernel[i * filter_size + j] = value;
            total_sum += value;
        }
    }

    for (size_t i = 0; i < filter_size; ++i) {
        for (size_t j = 0; j < filter_size; ++j) {
            kernel[i * filter_size + j] /= total_sum;
        }
    }
    return kernel;
}

float Gauss::gaussFunction(float x, float y, float sigma) {
    return std::exp(-(x*x+y*y)/(2*sigma*sigma)) / (2*M_PI*sigma*sigma);
}
