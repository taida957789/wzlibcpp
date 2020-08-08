#include <wz/WZ.h>
#include <wz/Directory.hpp>
#include <cassert>
#include <AES.h>
#include "File.hpp"
#include "Directory.hpp"
#include "Keys.hpp"

wz::File::File(const std::initializer_list<u8>& new_iv, const char *path)
    : reader(Reader(path)), root(new Node(reader)), key(new u8[0x10000]), iv(nullptr) {
    iv = new u8[4];
    memcpy(iv, new_iv.begin(), 4);
    key = new u8[0x10000];
    init_key();
    reader.set_key(key);
}

wz::File::File(const u8* new_iv, const wchar_t *path)
    : reader(Reader(path)), root(new Node(reader)), key(new u8[0x10000]), iv(nullptr) {
    iv = new u8[4];
    memcpy(iv, new_iv, 4);
    key = new u8[0x10000];
    init_key();
    reader.set_key(key);
}

bool wz::File::parse() {
    auto magic = reader.readString(4);
    if (magic != L"PKG1") return false;

    auto fileSize = reader.read<u64>();
    auto startAt = reader.read<u32>();

    auto copyright = reader.readString();

    reader.set_position(startAt);

    auto encryptedVersion = reader.read<i16>();

    i16 fileVersion = -1;

    for (int i = 0; i < 0x7FFF; ++i) {
        fileVersion = static_cast<decltype(fileVersion)>(i);
        u32 versionHash = wz::GetVersionHash(encryptedVersion, fileVersion);

        if (versionHash != 0) {
            desc.start = startAt;
            desc.hash = versionHash;
            desc.version = fileVersion;

            auto prevPosition = reader.get_position();

            if (!parse_directories(nullptr)) {
                reader.set_position(prevPosition);
                continue;
            } else {
                if (root) {
                    reader.set_position(prevPosition);
                    parse_directories(root);
                }
                return true;
            }
        }
    }

    return false;

}

bool wz::File::parse_directories(wz::Node *node) {
    auto entryCount = reader.readCompressedInt();

    for (int i = 0; i < entryCount; ++i) {
        auto type = reader.readByte();
        size_t prevPos = 0;
        std::wstring name;

        if (type == 1) {
            reader.skip(sizeof(i32) + sizeof(u16));

            get_wz_offset();
            continue;
        } else if (type == 2) {
            i32 stringOffset = reader.read<i32>();
            type = reader.readWzStringFromOffset<u8>(desc.start + stringOffset, name);
        } else if (type == 3 || type == 4) {
            name = reader.readWzString();
        } else {
            assert(0);
        }

        i32 size = reader.readCompressedInt();
        i32 checksum = reader.readCompressedInt();
        u32 offset = get_wz_offset();

        if (node == nullptr && offset >= reader.size())
            return false;

        if (type == 3) {
            if (node != nullptr) {
                auto* dir = new Directory(reader, false, size, checksum, offset);
                node->appendChild({name.begin(), name.end()}, dir);
            }
        } else {
            if (node != nullptr) {
                auto* dir = new Directory(reader, true, size, checksum, offset);
                node->appendChild({name.begin(), name.end()}, dir);
            } else {
                prevPos = reader.get_position();
                reader.set_position(offset);

                if (!reader.is_wz_image())
                    return false;

                reader.set_position(prevPos);
            }
        }
    }

    if (node != nullptr) {
        for (auto& it : *node) {
            for (auto child : it.second) {
                auto* dir = dynamic_cast<Directory*>(child);

                if (dir != nullptr) {
                    if (!dir->is_image()) {
                        reader.set_position(dir->get_offset());
                        parse_directories(dir);
                    }
                }
            }
        }
    }

    return true;
}

u32 wz::File::get_wz_offset() {
    u32 offset = static_cast<u32>(reader.get_position());
    offset = ~(offset - desc.start);
    offset *= desc.hash;
    offset -= wz::OffsetKey;
    offset = (offset << (offset & 0x1Fu)) | (offset >> (32 - (offset & 0x1Fu)));
    u32 encryptedOffset = reader.read<u32>();
    offset ^= encryptedOffset;
    offset += desc.start * 2;
    return offset;
}

wz::Node* wz::File::get_root() const {
    return root;
}

wz::Reader& wz::File::ref_reader() {
    return reader;
}

void wz::File::init_key() {
    AES AESGen;
    u8 biv[16];

    for (int i = 0; i < 16; i += 4) {
        memmove(biv + i, iv, 4);
    }

    AESGen.SetParameters(256, 128);
    AESGen.StartEncryption(wz::AesKey2);
    AESGen.EncryptBlock(biv, key);

    for (int i = 16; i < 0x10000; i += 16) {
        AESGen.EncryptBlock(key + i - 16, key + i);
    }
}

u8 *wz::File::get_key() const {
    return key;
}
