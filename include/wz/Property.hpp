#pragma once

#include <cmath>
#include <array>
#include <utility>
#include <iostream>
#include <zlib.h>
#include "Node.hpp"
#include "Keys.hpp"

namespace wz {
    template<typename T>
    class Property : public Node {
    public:
        explicit Property(const Type& new_type, File* root_file) : Node(new_type, root_file) {
            assert(is_property());
        }

        explicit Property(const Type& new_type, File* root_file, T new_data)
            : data(new_data), Node(new_type, root_file) {
            assert(is_property());
        };

        void set(T new_data) {
            data = new_data;
        }

        [[maybe_unused]]
        const T& get() const {
            return data;
        }

#define DEFAULT {0,0,0,0}

        [[nodiscard]] [[maybe_unused]]
        std::vector<u8> get_png(std::array<u8, 4> iv = DEFAULT) {
            assert(type == Type::Canvas);
            WzCanvas canvas = get();

            std::vector<u8> data_stream;
            reader->set_position(canvas.offset);
            size_t end_offset = reader->get_position() + canvas.size;

            if (*reinterpret_cast<i32*>(iv.data()) != 0) {
                auto wz_key = MutableKey(iv, get_trimmed_user_key());

                while (reader->get_position() < end_offset) {
                    auto block_size = reader->read<i32>();
                    for (size_t i = 0; i < block_size; ++i) {
                        auto n = wz_key[i];
                        data_stream.push_back(
                                static_cast<u8>(reader->read_byte() ^ n));
                    }
                }
            } else {
                auto wz_key = get_key();

                while (reader->get_position() < end_offset) {
                    auto block_size = reader->read<i32>();
                    for (size_t i = 0; i < block_size; ++i) {
                        auto n = wz_key[i];
                        data_stream.push_back(
                                static_cast<u8>(reader->read_byte() ^ n));
                    }
                }
            }

            size_t len;
            u8* uncompressed = new u8[8192];
            uncompress(uncompressed, &len, data_stream.data(), data_stream.size());
            std::vector<u8> buffer(canvas.uncompressed_size);
            memcpy(buffer.data(), uncompressed, canvas.uncompressed_size);
            delete[] uncompressed;

            switch (canvas.format + canvas.format2) {
                case 1: {
                    size_t size = canvas.uncompressed_size * 2;
                    std::vector<u8> result(size);
                    for (int i = 0; i < buffer.size(); ++i) {
                        auto b = buffer[i] & 0x0Fu;
                        b |= b << 4u;
                        result[i * 2] = b;
                        auto g = buffer[i] & 0xF0u;
                        g |= g >> 4u;
                        result[i * 2 + 1] = g;
                    }

                    return result;
                }
            }

            return {};
        }

    private:
        T data;
    };
}
