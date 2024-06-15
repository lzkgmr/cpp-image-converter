#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <iostream>

using namespace std;

namespace img_lib {

  PACKED_STRUCT_BEGIN BitmapFileHeader {
      uint16_t file_type{ 0x4D42 };          // File type always BM which is 0x4D42
      uint32_t file_size{ 0 };               // Size of the file (in bytes)
      uint16_t reserved1{ 0 };               // Reserved, always 0
      uint16_t reserved2{ 0 };               // Reserved, always 0
      uint32_t offset_data{ 0 };
  }
  PACKED_STRUCT_END

      PACKED_STRUCT_BEGIN BitmapInfoHeader {
  uint32_t size{ 0 };                      // Size of this header (in bytes)
  int32_t width{ 0 };                      // width of bitmap in pixels
  int32_t height{ 0 };                     // width of bitmap in pixels
  uint16_t planes{ 1 };                    // No. of planes for the target device, this is always 1
  uint16_t bit_count{ 0 };                 // No. of bits per pixel
  uint32_t compression{ 0 };               // 0 or 3 - uncompressed. THIS PROGRAM CONSIDERS ONLY UNCOMPRESSED BMP images
  uint32_t size_image{ 0 };                // 0 - for uncompressed images
  int32_t x_pixels_per_meter{ 0 };
  int32_t y_pixels_per_meter{ 0 };
  uint32_t colors_used{ 0 };               // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
  uint32_t colors_important{ 0 };
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
  return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
  BitmapFileHeader file_header;
  BitmapInfoHeader bmp_info_header;

  bmp_info_header.width = image.GetWidth();
  bmp_info_header.height = image.GetHeight();
  bmp_info_header.bit_count = 24;
  bmp_info_header.x_pixels_per_meter = 11811;
  bmp_info_header.y_pixels_per_meter = 11811;
  bmp_info_header.colors_important = 16777216;


  const auto stride = GetBMPStride(image.GetWidth());
  file_header.file_size = sizeof(file_header) + sizeof(bmp_info_header) + stride * bmp_info_header.height;
  file_header.offset_data = sizeof(file_header) + sizeof(bmp_info_header);
  bmp_info_header.size_image = bmp_info_header.height * stride;

  bmp_info_header.size = 40;

  ofstream output{file, ios_base::binary};
  if(!output) {
    return false;
  }

  output.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
  output.write(reinterpret_cast<const char*>(&bmp_info_header), sizeof(bmp_info_header));

  std::vector<char> padding(stride - bmp_info_header.width * 3);
  for(int32_t y = bmp_info_header.height - 1; y >= 0; --y) {
    for(int32_t x = 0; x < bmp_info_header.width; ++x) {
      const Color color = image.GetPixel(x, y);
      output.write(reinterpret_cast<const char*>(&color.b), sizeof(color.b));
      output.write(reinterpret_cast<const char*>(&color.g), sizeof(color.g));
      output.write(reinterpret_cast<const char*>(&color.r), sizeof(color.r));
    }
    if(!padding.empty()) {
      output.write(padding.data(), padding.size());
    }
  }
  return output.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
  ifstream input{file, ios_base::binary};

  if(!input) {
    return {};
  }

  BitmapFileHeader file_header;
  BitmapInfoHeader bmp_info_header;

  input.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
  input.read(reinterpret_cast<char*>(&bmp_info_header), sizeof(bmp_info_header));

  if(file_header.file_type != 0x4D42) {  // Ensure the file is BMP
    return {};
  }

  if(bmp_info_header.bit_count != 24) {  // Ensure the bitmap is 24 bits (1 byte for R, G, B)
    return {};
  }

  if(bmp_info_header.compression != 0) {  // Ensure the bitmap is not compressed
    return {};
  }


  Image image(bmp_info_header.width, bmp_info_header.height, Color::Black());

  vector<char> padding(GetBMPStride(bmp_info_header.width) - bmp_info_header.width * 3);

  for(int32_t y = bmp_info_header.height - 1; y >= 0; --y) {
    for(int32_t x = 0; x < bmp_info_header.width; ++x) {
      Color color;

      input.read(reinterpret_cast<char*>(&color.b), sizeof(color.b));
      input.read(reinterpret_cast<char*>(&color.g), sizeof(color.g));
      input.read(reinterpret_cast<char*>(&color.r), sizeof(color.r));

      image.SetPixel(x, y, color);
    }

    if(!padding.empty()) {
      input.read(padding.data(), padding.size());
    }
  }

  return image;
}

}  // namespace img_lib