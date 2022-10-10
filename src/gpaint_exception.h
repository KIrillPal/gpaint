#pragma  once
#include <stdexcept>

#define GPAINT_EXCEPTION(...) {                   \
    char GpaintErrorMessage[64];                  \
    snprintf(GpaintErrorMessage, 64, __VA_ARGS__);\
    throw std::runtime_error(GpaintErrorMessage); \
}