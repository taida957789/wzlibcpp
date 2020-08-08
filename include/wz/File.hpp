#pragma once

#include "Node.hpp"
#include "Reader.hpp"
#include "Wz.hpp"

namespace wz {
    class File final {

    public:

        explicit File(const std::initializer_list<u8>& new_iv, const char* path);
        explicit File(const u8* new_iv, const wchar_t* path);

        [[maybe_unused]]
        bool parse();

        [[nodiscard]]
        Node* get_root() const;

        [[nodiscard]]
        u8* get_key() const;

    private:

        u8* key;
        u8* iv;

        Node* root;

        Description desc {};

        Reader reader;

        bool parse_directories(Node* node);

        u32 get_wz_offset();

        void init_key();

    };
}