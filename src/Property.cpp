#include "Property.hpp"
#include "Types.hpp"

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
        printf("123");
    }
}