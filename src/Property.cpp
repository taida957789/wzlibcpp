#include "Property.hpp"
#include "Types.hpp"

static void *myalloc(void *q, unsigned n, unsigned m)
{
    (void)q;
    return calloc(n, m);
}

static void myfree(void *q, void *p)
{
    (void)q;
    free(p);
}

static alloc_func zalloc = myalloc;
static free_func zfree = myfree;

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
        return pixel_stream;
    }
}