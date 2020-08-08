#ifndef __WZ_h__
#define __WZ_h__

#include "BinReader.h"
#include "Node.h"
#include "Directory.hpp"
#include "Types.hpp"

//////////////////////////////////////////////////////////////////////////


struct WzFileDesc {
    unsigned int m_Start;
    unsigned int m_Hash;
    short m_FileVersion;

    WzFileDesc() : m_Start(0), m_Hash(0), m_FileVersion(0) {}
};

//////////////////////////////////////////////////////////////////////////
const unsigned char WzGMSKeyIV[4] =
        {
                0x4D, 0x23, 0xC7, 0x2B
        };
const unsigned char WzKMSKeyIV[4] =
        {
                0xB9, 0x7D, 0x63, 0xE9
        };

[[deprecated]]
void WzGenKeys(const unsigned char* IV);

[[deprecated]]
void WzParseListFile(BinReader& reader, std::vector<std::wstring> &strList);

[[deprecated]]
bool WzParseFile(BinReader& reader, WzFileDesc& fdesc, wz::Node* root);

[[deprecated]]
bool WzParseImage(BinReader& reader, const wz::Directory* pimg, wz::Node* root);

[[deprecated]]
void* WzRebuildCanvas(FileMapping& fm, const wz::WzCanvas& canvas, size_t& szBuf, size_t& bpp);

namespace wz {

    struct Description {
        u32 start;
        u32 hash;
        i16 version;
    };

    u32 GetVersionHash(i32 encryptedVersion, i32 realVersion);

    [[deprecated]]
    void initAES(const u8* iv);

}

#endif