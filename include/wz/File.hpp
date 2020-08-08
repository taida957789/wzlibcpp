#pragma once

#include "Node.hpp"
#include "Reader.hpp"
#include "Wz.hpp"

namespace wz {
    class File final {

    public:

        explicit File(const std::initializer_list<u8>& new_iv, const char* path);

#ifdef _WIN32
        explicit File(const std::initializer_list<u8>& new_iv, const wchar_t* path);
#endif

        [[maybe_unused]]
        bool parse();

        [[maybe_unused]] [[nodiscard]]
        Node* get_root() const;

        [[maybe_unused]] [[nodiscard]]
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