#pragma once

#include "Node.h"
#include "NumTypes.hpp"

namespace wz {
    class Directory : public Node {
    public:
        explicit Directory(Reader& from_file, bool img, int new_size, int new_checksum, unsigned int new_offset);

        [[deprecated]]
        void Set(bool img, int new_size, int new_checksum, unsigned int new_offset);

        [[nodiscard]]
        u32 get_offset() const;

        [[nodiscard]]
        bool is_image() const;

        bool parse(Node* node);

    private:
        bool image;
        int size;
        int checksum;
        unsigned int offset;
    };
}