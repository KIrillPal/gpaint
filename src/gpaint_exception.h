#ifndef GPAINT_EXCEPTION_H
#define GPAINT_EXCEPTION_H
#include <stdexcept>

#define GPAINT_EXCEPTION(...) {                    \
    char GpaintErrorMessage[128];                  \
    snprintf(GpaintErrorMessage, 128, __VA_ARGS__);\
    throw std::runtime_error(GpaintErrorMessage);  \
}
#endif