#pragma once
#include <cstdint>
#include <cstddef>

struct __attribute__((packed)) RGBColor {
    uint8_t B, G, R;

    RGBColor(uint8_t R, uint8_t G, uint8_t B) : R(R), G(G), B(B) {}
    RGBColor(unsigned hex_code) {
        *this = hex_code;
    }

    friend bool operator==(const RGBColor& first, const RGBColor& second) {
        return (first.R == second.R && 
                first.G == second.G && 
                first.B == second.B);
    }
    friend bool operator!=(const RGBColor& first, const RGBColor& second) {
        return !(first == second);
    }

    RGBColor& operator=(unsigned hex_code) {
        R = (hex_code & 0x00FF0000) >> 16;
        G = (hex_code & 0x0000FF00) >> 8;
        B = (hex_code & 0x000000FF);
        return *this;
    }
};

enum RGB {
    Null  = 0x00000000,
    Black = 0xFF000000,
    White = 0xFFFFFFFF,

    Red   = 0xFFFF0000,
    Green = 0xFF00FF00,
    Blue  = 0xFF0000FF,

    Cyan   = 0xFF00FFFF,
    Purple = 0xFFFF00FF,
    Yellow = 0xFFFFFF00,
};