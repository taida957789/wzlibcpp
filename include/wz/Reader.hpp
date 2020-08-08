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
        u8 read_byte();

        /*
         * read string until **null terminated**
         */
        [[nodiscard]]
        auto read_string() -> std::wstring;

        [[nodiscard]]
        auto read_string(const size_t& len) -> std::wstring;

        [[nodiscard]]
        i32 read_compressed_int();

        [[nodiscard]]
        std::wstring read_wz_string();

        std::wstring read_string_block(const size_t& offset);

        template<typename T> [[nodiscard]]
        T read_wz_string_from_offset(const size_t& offset, std::wstring& out) {
            auto prev = cursor;
            set_position(offset);
            auto result = read<T>();
            out = read_wz_string();
            set_position(prev);
            return result;
        }

        std::wstring read_wz_string_from_offset(const size_t& offset);

        [[nodiscard]]
        size_t get_position() const;

        void set_position(const size_t& size);

        mio::mmap_source::size_type size() const;

        [[nodiscard]]
        bool is_wz_image();

        void set_key(u8* new_key);

    private:

        u8* key = nullptr;

        size_t cursor = 0;

        mio::mmap_source mmap;

        explicit Reader() = default;

        friend class Node;


    };
}
