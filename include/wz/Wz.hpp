#ifndef __WZ_h__
#define __WZ_h__

#include "Types.hpp"

//////////////////////////////////////////////////////////////////////////

namespace wz {
    enum class Type : u8 {
        NotSet = 0x00,
        Directory = 0x10,
        Image = 0x20,
        Property = 0x30,
    };

    namespace keys {
        [[maybe_unused]]
        const unsigned char gms[4] = {
            0x4D, 0x23, 0xC7, 0x2B
        };

        [[maybe_unused]]
        const unsigned char kms[4] = {
            0xB9, 0x7D, 0x63, 0xE9
        };
    }
}

//////////////////////////////////////////////////////////////////////////


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