#include <cassert>
#include "wz/Reader.hpp"
#include "Keys.hpp"

wz::Reader::Reader(const char *file_path)
        : cursor(0) {
    std::error_code error_code;
    mmap = mio::make_mmap_source<decltype(file_path)>(file_path, error_code);
}

wz::Reader::Reader(const wchar_t *file_path)
        : cursor(0) {
    std::error_code error_code;
    mmap = mio::make_mmap_source<decltype(file_path)>(file_path, error_code);
}

u8 wz::Reader::readByte() {
    return mmap[cursor++];
}

auto wz::Reader::readString() -> std::wstring {
    std::wstring result {};

    while (true) {
        auto c = static_cast<char>(readByte());
        if (!c) break;
        result.push_back(c);
    }

    return result;
}

auto wz::Reader::readString(const size_t &len) -> std::wstring {
    std::wstring result {};

    for (int i = 0; i < len; ++i) {
        result.push_back(readByte());
    }

    return result;
}

void wz::Reader::set_position(const size_t& size) {
    cursor = size;
}

size_t wz::Reader::get_position() const {
    return cursor;
}

void wz::Reader::skip(const size_t& size) {
    cursor += size;
}

i32 wz::Reader::readCompressedInt() {
    i32 result = static_cast<i32>(read<i8>());
    if (result == INT8_MIN) return read<i32>();
    return result;
}

std::wstring wz::Reader::readWzString() {
    auto len8 = read<i8>();

    if (len8 == 0) return {};

    i32 len;

    if (len8 > 0) {
        u16 mask = 0xAAAA;

        len = len8 == 127 ? read<i32>() : len8;

        if (len <= 0) {
            return {};
        }

        std::wstring result{};

        for (int i = 0; i < len; ++i) {
            wchar_t encryptedChar = read<u16>();
            encryptedChar ^= mask;
            encryptedChar ^= *reinterpret_cast<wchar_t*>(key + 2 * i);
            result.push_back(encryptedChar);
            mask++;
        }

        return result;
    }

    u8 mask = 0xAA;

    if (len8 == -128) {
        len = read<i32>();
    } else {
        len = -len8;
    }

    if (len <= 0) {
        return {};
    }

    std::wstring result {};

    for (int n = 0; n < len; ++n) {
        u8 encryptedChar = readByte();
        encryptedChar ^= mask;
        encryptedChar ^= key[n];
        result.push_back(static_cast<wchar_t>(encryptedChar));
        mask++;
    }

    return result;
}

mio::mmap_source::size_type wz::Reader::size() {
    return mmap.size();
}

bool wz::Reader::is_wz_image() {
    return !(read<u8>() != 0x73
             || readWzString() != L"Property"
             || read<u16>() != 0);
}

std::wstring wz::Reader::readStringBlock(const size_t& offset) {
    switch (read<u8>()) {
        case 0: [[fallthrough]];
        case 0x73:
            return readString();
        case 1: [[fallthrough]];
        case 0x1B:
            return readWzStringFromOffset(offset);
        default: {
            assert(0);
        }
    }
    return {};
}

std::wstring wz::Reader::readWzStringFromOffset(const size_t &offset) {
    auto prev = get_position();
    set_position(offset);
    auto result = readWzString();
    set_position(prev);
    return result;
}

void wz::Reader::set_key(u8 *new_key) {
    key = new_key;
}
