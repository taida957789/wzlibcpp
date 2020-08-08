#pragma once

#include "Node.h"
#include "NumTypes.hpp"

namespace wz {
    class Directory : public Node {
    public:
        explicit Directory(bool img, int new_size, int new_checksum, unsigned int new_offset);

        [[deprecated]]
        void Set(bool img, int new_size, int new_checksum, unsigned int new_offset);

        [[nodiscard]]
        u32 GetOffset() const;

        [[nodiscard]]
        bool IsImage() const;

    public:

        [[deprecated]]
        static Directory *New();

    private:
        bool image;
        int size;
        int checksum;
        unsigned int offset;
    };
}