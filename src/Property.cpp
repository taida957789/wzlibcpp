#include "Property.hpp"
#include "Types.hpp"

enum PixelFormat
{
    FORMAT_RGBA8 = 5,
    FORMAT_RGBA4444=6,
};

void convert_argb4444_to_rgba4444(std::vector<u8>& pixels) {
    for (unsigned int i = 0; i < pixels.size(); i += 2) {
        unsigned char r = (pixels[i] & 0xF0) >> 4;  // Extract 4 bits for red
        unsigned char g = (pixels[i] & 0x0F) << 4 | (pixels[i + 1] & 0xF0) >> 4;  // Extract 4 bits for green and arrange
        unsigned char b = (pixels[i + 1] & 0x0F) << 4;  // Extract 4 bits for blue
        pixels[i] = (r << 4) | (g >> 4);  // Rearrange bytes to form new pixel
        pixels[i + 1] = (g << 4) | b;
    }
}

//直接获取纹理格式数据，可以直接导入到游戏引擎使用
template <>
std::vector<u8> wz::Property<wz::WzCanvas>::get_raw_data(std::array<u8, 4> iv)
{
    WzCanvas canvas = get();

    std::vector<u8> data_stream;
    reader->set_position(canvas.offset);
    size_t end_offset = reader->get_position() + canvas.size;
    unsigned long uncompressed_len = canvas.uncompressed_size;
    u8 *uncompressed = new u8[uncompressed_len];
    if (!canvas.is_encrypted)
    {
        for (size_t i = 0; i < canvas.size; ++i)
        {
            data_stream.push_back(reader->read_byte());
        }
        int len = data_stream.size();
        uncompress(uncompressed, &uncompressed_len, data_stream.data(), data_stream.size());
    }
    else
    {
        auto wz_key = get_key();

        while (reader->get_position() < end_offset)
        {
            auto block_size = reader->read<i32>();
            for (size_t i = 0; i < block_size; ++i)
            {
                auto n = wz_key[i];
                data_stream.push_back(
                    static_cast<u8>(reader->read_byte() ^ n));
            }
        }
        uncompress(uncompressed, (unsigned long *)&uncompressed_len, data_stream.data(), data_stream.size());
    }

    std::vector<u8> pixel_stream(uncompressed, uncompressed + uncompressed_len);
    convert_argb4444_to_rgba4444(pixel_stream);
    switch (canvas.format)
    {
    case 1: // 16位argb4444
        break;
    case 2:
        break;
    default:
        break;
    }
    return pixel_stream;
}
