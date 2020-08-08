#pragma once

#include <mio/mmap.hpp>
#include "NumTypes.hpp"

namespace wz {
    class Reader final {
    public:
        explicit Reader(const char* file_path);
        explicit Reader(const wchar_t* file_path);

        template<typename T> [[nodiscard]]
        T read() {
            auto result = *reinterpret_cast<T*>(&mmap[cursor]);
            cursor += sizeof(decltype(result));
            return result;
        }

        void skip(const size_t& size);

        [[nodiscard]]
        u8 readByte();

        [[nodiscard]]
        auto readString() -> std::wstring;

        [[nodiscard]]
        auto readString(const size_t& len) -> std::wstring;

        [[nodiscard]]
        i32 readCompressedInt();

        [[nodiscard]]
        std::wstring readWzString();

        std::wstring readStringBlock(const size_t& offset);

        template<typename T> [[nodiscard]]
        T readWzStringFromOffset(const size_t& offset, std::wstring& out) {
            auto prev = cursor;
            set_position(offset);
            auto result = read<T>();
            out = readWzString();
            set_position(prev);
            return result;
        }

        std::wstring readWzStringFromOffset(const size_t& offset);

        [[nodiscard]]
        size_t get_position() const;

        void set_position(const size_t& size);

        mio::mmap_source::size_type size();

        [[nodiscard]]
        bool is_wz_image();

        void set_key(u8* new_key);

    private:

        u8* key;

        size_t cursor = 0;

        mio::mmap_source mmap;

        explicit Reader() = default;

        friend class Node;


    };
}
