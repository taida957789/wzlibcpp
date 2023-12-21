#include "Property.hpp"
#include "Types.hpp"

enum PixelFormat
{
    FORMAT_RGBA8 = 5,
    FORMAT_RGBA4444 = 6,
};

void convert_argb4444_to_rgba4444(std::vector<u8> &pixelData)
{
    for (int i = 0; i < pixelData.size(); i += 2)
    {
        u16 argb4444Pixel = (u16)((pixelData[i + 1] << 8) | pixelData[i]);
        u16 a = (u16)((argb4444Pixel & 0xF000) >> 12);
        u16 r = (u16)((argb4444Pixel & 0x0F00) >> 8);
        u16 g = (u16)((argb4444Pixel & 0x00F0) >> 4);
        u16 b = (u16)(argb4444Pixel & 0x000F);

        // Convert to RGBA4444
        u16 rgba4444Pixel = (u16)((r << 12) | (g << 8) | (b << 4) | a);

        // Store in RGBA4444 format
        pixelData[i] = (u8)(rgba4444Pixel & 0xFF);
        pixelData[i + 1] = (u8)((rgba4444Pixel >> 8) & 0xFF);
    }
}

// 直接获取纹理格式数据，可以直接导入到游戏引擎使用
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
    switch (canvas.format)
    {
    case 1: // 16位argb4444
        break;
    case 2:
        break;
    default:
        break;
    }
    convert_argb4444_to_rgba4444(pixel_stream);
    return pixel_stream;
}
