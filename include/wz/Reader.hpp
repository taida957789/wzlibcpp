#pragma once

#include <mio/mio.hpp>
#include "NumTypes.hpp"

namespace wz {
    class Reader final {
    public:

        explicit Reader(const char* file_path);

        template<typename T>
        T read() {
            auto result = *reinterpret_cast<T*>(&mmap[cursor]);
            cursor += sizeof(decltype(result));
            return result;
        }

        u8 readByte();

    private:

        size_t cursor;

        mio::mmap_source mmap;


    };
}
