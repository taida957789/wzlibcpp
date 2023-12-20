#include "Property.hpp"
#include "Types.hpp"

enum PixelFormat
{
    FORMAT_RGBA8 = 5,

};

template <>
std::vector<u8> wz::Property<wz::WzCanvas>::get_png(std::array<u8, 4> iv)
{
    WzCanvas canvas = get();

    std::vector<u8> data_stream;
    reader->set_position(canvas.offset);
    size_t end_offset = reader->get_position() + canvas.size;
    std::vector<u8> pixel_stream;
    unsigned long uncompressed_len = canvas.uncompressed_size;
    // u8 *uncompressed = new u8[uncompressed_len];
    u8 uncompressed[uncompressed_len] = {0};
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
        // for (size_t i = data_stream.size(); i < canvas.size; ++i)
        // {
        //     data_stream.push_back(0);
        // }
        uncompress(uncompressed, (unsigned long *)&uncompressed_len, data_stream.data(), data_stream.size());
    }

    size_t pixel_data_len = canvas.width * canvas.height * 2;
    for (size_t i = pixel_stream.size(); i < pixel_data_len; ++i)
    {
        pixel_stream.push_back(uncompressed[i]);
    }

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
