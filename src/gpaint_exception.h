#include <stdexcept>

char GpaintErrorMessage[64];

#define GPAINT_EXCEPTION(...) {                   \
    snprintf(GpaintErrorMessage, 64, __VA_ARGS__);\
    throw std::runtime_error(GpaintErrorMessage); \
}