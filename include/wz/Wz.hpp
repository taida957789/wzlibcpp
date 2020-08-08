#ifndef __WZ_h__
#define __WZ_h__

#include "Node.hpp"
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

namespace wz {

    struct Description {
        u32 start;
        u32 hash;
        i16 version;
    };

    u32 get_version_hash(i32 encryptedVersion, i32 realVersion);

    [[deprecated]]
    void initAES(const u8* iv);

}

#endif