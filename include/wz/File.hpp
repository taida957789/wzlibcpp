#pragma once

#include "Node.hpp"
#include "Reader.hpp"
#include "Wz.hpp"
#include "Keys.hpp"

namespace wz
{
    class File final
    {

    public:
        [[maybe_unused]] explicit File(const std::initializer_list<u8> &new_iv, unsigned char *wz_buf, unsigned int wz_size);

        [[maybe_unused]] explicit File(const std::initializer_list<u8> &new_iv, const char *path);

        [[maybe_unused]] explicit File(u8 *new_iv, const char *path);

        ~File();

        [[maybe_unused]] bool parse(const std::wstring &name = L"");

        [[maybe_unused]] [[nodiscard]] Node *get_root() const;
        Node &get_child(const std::wstring &name);

        MutableKey key;

    private:
        // u8* key;
        u8 *iv;

        Node *root;

        Description desc{};

        Reader reader;

        bool parse_directories(Node *node);

        u32 get_wz_offset();

        void init_key();

        friend class Node;
    };
}