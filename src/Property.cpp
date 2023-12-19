#include "Property.hpp"
#include "Types.hpp"

std::vector<u8> get_pixel_data_Bgra4444(vector<u8> rawData, int width, int height)
{
    u8 argb[width * height * 4];
    int p;
    for (int i = 0; i < rawData.size(); i++)
    {
        p = rawData[i] & 0x0F;
        p |= (p << 4);
        argb[i * 2] = p;
        p = rawData[i] & 0xF0;
        p |= (p >> 4);
        argb[i * 2 + 1] = p;
    }
    return vector<u8>(argb, argb + width * height * 4);
}

template <>
std::vector<u8> wz::Property<wz::WzCanvas>::get_png(std::array<u8, 4> iv)
{
    WzCanvas canvas = get();

    std::vector<u8> data_stream;
    reader->set_position(canvas.offset);
    size_t end_offset = reader->get_position() + canvas.size;
    auto wz_key1 = MutableKey(iv, get_trimmed_user_key());
    if (*reinterpret_cast<i32 *>(iv.data()) != 0)
    {
        auto wz_key = MutableKey(iv, get_trimmed_user_key());

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
        for (size_t i = data_stream.size(); i < canvas.size; ++i)
        {
            data_stream.push_back(0);
        }

        size_t uncompressed_len = data_stream.size() * 4;
        u8 uncompressed[uncompressed_len];
        uncompress(uncompressed, (unsigned long *)&uncompressed_len, data_stream.data(), data_stream.size());

        size_t pixel_data_len = canvas.width * canvas.height * 2;
        std::vector<u8> pixel_stream;
        for (size_t i = pixel_stream.size(); i < pixel_data_len; ++i)
        {
            pixel_stream.push_back(uncompressed[i]);
        }
        switch (canvas.format)
        {
        case 1: //16位argb4444
            pixel_stream=get_pixel_data_Bgra4444(pixel_stream,canvas.width,canvas.height);
            break;
        case 2:

            break;

        default:
            break;
        }
        return pixel_stream;
    }
}

