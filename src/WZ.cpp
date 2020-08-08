#include <assert.h>
#include <malloc.h>
#include <zlib.h>
#include <wz/Property.hpp>
#include <wz/Directory.hpp>
#include <wz/Keys.hpp>
#include "wz/Wz.hpp"
#include "AES.h"

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
static void WzDecryptString(const std::wstring &stringToDecrypt, std::wstring &outputString) {
    assert(outputString.size() >= stringToDecrypt.size());
    size_t szToDec = stringToDecrypt.size();
    for (size_t i = 0; i < szToDec; i++)
        outputString[i] = stringToDecrypt[i] ^ *(wchar_t *) (wz::WzKey + 2 * i);
}

static unsigned int WzGetVersionHash(int encver, int realver) {
    int EncryptedVersionNumber = encver;
    int VersionNumber = realver;
    int VersionHash = 0;
    int DecryptedVersionNumber = 0;
    std::wstring VersionNumberStr;
    int a = 0, b = 0, c = 0, d = 0, l = 0;

    wchar_t stmp[16];
    swprintf(stmp, L"%d", VersionNumber);
    VersionNumberStr = stmp;

    l = VersionNumberStr.size();
    for (int i = 0; i < l; i++) {
        VersionHash = (32 * VersionHash) + (int) VersionNumberStr[i] + 1;
    }
    a = (VersionHash >> 24) & 0xFF;
    b = (VersionHash >> 16) & 0xFF;
    c = (VersionHash >> 8) & 0xFF;
    d = VersionHash & 0xFF;
    DecryptedVersionNumber = (0xFF ^ a ^ b ^ c ^ d);

    if (EncryptedVersionNumber == DecryptedVersionNumber) return (unsigned int) VersionHash;
    else return 0;
}

void WzGenKeys(const unsigned char *IV) {
    AES AESGen;
    unsigned char BigIV[16];

    for (int i = 0; i < 16; i += 4) {
        memmove(BigIV + i, IV, 4);
    }

    AESGen.SetParameters(256, 128);
    AESGen.StartEncryption(wz::AesKey2);
    AESGen.EncryptBlock(BigIV, wz::WzKey);

    for (int i = 16; i < 0x10000; i += 16) {
        AESGen.EncryptBlock(wz::WzKey + i - 16, wz::WzKey + i);
    }
}

u32 wz::get_version_hash(i32 encryptedVersion, i32 realVersion) {
    i32 versionHash = 0;
    auto versionString = std::to_string(realVersion);

    auto len = versionString.size();

    for (int i = 0; i < len; ++i) {
        versionHash = (32 * versionHash) + static_cast<i32>(versionString[i]) + 1;
    }

#define HASHING(V, S) ((V >> S##u) & 0xFFu)
#define AUTO_HASH(V) (0xFFu ^ HASHING(V, 24) ^ HASHING(V, 16) ^ HASHING(V, 8) ^ V & 0xFFu)

    i32 decryptedVersionNumber = AUTO_HASH(static_cast<u32>(versionHash));

    if (encryptedVersion == decryptedVersionNumber) {
        return static_cast<u32>(versionHash);
    }

    return 0;

}

