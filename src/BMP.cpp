#include <cstdio>
#include "BMP.h"
#include "gpaint_exception.h"

 void BMPReader::loadFromFile(const char* path, Image& image) {
    BMPFileHeader  file_header;
    BMPInfoHeader  info_header;
    BMPColorHeader color_header;

    FILE * file = fopen(path, "rb");
    if (file == nullptr)
        GPAINT_EXCEPTION("Couldn't open file \"%s\" to load", path);

    if (!readBMPFileHeader (file, &file_header))
        GPAINT_EXCEPTION("Couldn't open file \"%s\" to load", path);

    if (!readBMPInfoHeader (file, &info_header))
        GPAINT_EXCEPTION("Invalid BMP info header format of file \"%s\"", path);

    if (info_header.size == sizeof(BMPInfoHeader) + sizeof(BMPColorHeader)) {
        if (!readBMPColorHeader(file, &color_header))
            GPAINT_EXCEPTION("Invalid BMP color header format of file \"%s\"", path);
    }
    fseek(file, file_header.offset_data, 0);

    image = Image(info_header.width, info_header.height);
    if (color_header.color_space_type == COLOR_SPACE::sRGB)
    {
        if (info_header.bit_count != 24)
            GPAINT_EXCEPTION("Invalid color format of file \"%s\"", path);
        if (!readImageRGB(file, image, info_header))
            GPAINT_EXCEPTION("Invalid BMP meta of file \"%s\". Failed to load data.", path);
    }
    else {
        GPAINT_EXCEPTION("File \"%s\" has incorrect color space code: %X",
                         path, color_header.color_space_type);
    }
}

void BMPReader::saveToFile(const char* path, Image& image, uint32_t color_space_type) {
    BMPFileHeader  file_header;
    BMPInfoHeader  info_header;
    BMPColorHeader color_header;
    char errorMessage[64];

    FILE * file = fopen(path, "wb");
    if (file == nullptr)
        GPAINT_EXCEPTION("Couldn't open file \"%s\" to save", path);

    makesRGBHeaders(file_header, info_header, color_header, image);

    if (fwrite(&file_header, sizeof(file_header), 1, file) != 1)
        GPAINT_EXCEPTION("Couldn't save file header of file \"%s\"", path);

    if (fwrite(&info_header, sizeof(info_header), 1, file) != 1)
        GPAINT_EXCEPTION("Couldn't save info header of file \"%s\"", path);

    if (color_space_type != COLOR_SPACE::sRGB)
    {
        if (fwrite(&color_header, sizeof(color_header), 1, file) != 1)
            GPAINT_EXCEPTION("Couldn't save color header of file \"%s\"", path);
    }

    if (!writeImageRGB(file, image))
        GPAINT_EXCEPTION("Couldn't write image data to the file \"%s\"", path);
}

bool BMPReader::readBMPFileHeader(FILE* file, BMPFileHeader* header) {
    if (fread(header, sizeof(BMPFileHeader), 1, file) != 1) return false;
    if (header->file_type != 0x4D42) return false;
    if (header->reserved1 != 0) return false;
    if (header->reserved2 != 0) return false;
    return true;
}

bool BMPReader::readBMPInfoHeader(FILE* file, BMPInfoHeader* header) {
    if (fread(header, sizeof(BMPInfoHeader), 1, file) != 1) return false;
    if (header->bit_count & 7) return false;
    return true;
}

bool BMPReader::readBMPColorHeader (FILE* file, BMPColorHeader* header) {
    if (fread(header, sizeof(BMPColorHeader), 1, file) != 1) return false;
    return true;
}

bool BMPReader::readOffset(FILE *file, size_t offset) {
    const int MAX_BUFFER = 64;
    uint8_t buffer[MAX_BUFFER];
    while (offset > 0) {
        size_t buf_size = (offset < MAX_BUFFER ? offset : MAX_BUFFER);
        if (fread(buffer, buf_size, 1, file) != 1) {
            return false;
        }
    }
    return true;
}

bool BMPReader::readImageRGB(FILE* file, Image& image, BMPInfoHeader bmp_info) {
    size_t byte_count  = sizeof(RGBColor);
    size_t array_size  = bmp_info.width * bmp_info.height;
    RGBColor* img_data = image.GetPixelArray()[0];

    if (fread(img_data, byte_count, array_size, file) != array_size) {
        return false;
    }
    printf("%02X%02X%02X\n", img_data->R, img_data->G, img_data->B);
    return true;
}

bool BMPReader::writeImageRGB(FILE* file, Image& image) {
    size_t byte_count  = sizeof(RGBColor);
    size_t array_size  = image.GetWidth() * image.GetHeight();
    RGBColor* img_data = image.GetPixelArray()[0];

    if (fwrite(img_data, byte_count, array_size, file) != array_size) {
        return false;
    }
    return true;
}

void BMPReader::makesRGBHeaders(BMPFileHeader &fh, BMPInfoHeader &ih, BMPColorHeader &ch, const Image& image) {
    ih.size = sizeof(BMPInfoHeader);
    ih.width  = image.GetWidth();
    ih.height = image.GetHeight();
    ih.bit_count = sizeof(RGBColor) << 3;

    fh.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    fh.file_size   = fh.offset_data + ih.width * ih.height * sizeof(RGBColor);
    fh.file_type   = 0x4D42;
}