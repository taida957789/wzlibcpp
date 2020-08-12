#pragma once

#include "Node.hpp"
#include "Reader.hpp"
#include "Wz.hpp"
#include "Keys.hpp"

namespace wz {
    class File final {

    public:

        explicit File(const std::initializer_list<u8>& new_iv, const char* path);

        explicit File(u8* new_iv, const char* path);

#ifdef _WIN32
        explicit File(const std::initializer_list<u8>& new_iv, const wchar_t* path);
        
        explicit File(u8* new_iv,, const wchar_t* path);
#endif

        ~File();

        [[maybe_unused]]
        bool parse();

        [[maybe_unused]] [[nodiscard]]
        Node* get_root() const;

        MutableKey key;

    private:

        // u8* key;
        u8* iv;

        Node* root;

        Description desc {};

        Reader reader;

        bool parse_directories(Node* node);

        u32 get_wz_offset();

        void init_key();

        friend class Node;

    };
}