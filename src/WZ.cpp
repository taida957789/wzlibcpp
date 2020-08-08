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

static void WzReadString(BinReader &reader, std::wstring &outputString) {
    int smallLength = reader.ReadInt8();

    if (smallLength == 0) {
        return;
    }

    int length;

    if (smallLength > 0)    // Unicode
    {
        wchar_t mask = 0xAAAA;
        if (smallLength == 127) {
            length = reader.ReadInt32();
        } else {
            length = smallLength;
        }
        if (length <= 0) {
            return;
        }

        for (int i = 0; i < length; i++) {
            wchar_t encryptedChar = reader.ReadUInt16();
            encryptedChar ^= mask;
            encryptedChar ^= *(wchar_t *) (wz::WzKey + 2 * i);
            outputString.push_back(encryptedChar);
            mask++;
        }
    } else    // ASCII
    {
        unsigned char mask = 0xAA;
        if (smallLength == -128) {
            length = reader.ReadInt32();
        } else {
            length = -smallLength;
        }
        if (length <= 0) {
            return;
        }

        for (int i = 0; i < length; i++) {
            unsigned char encryptedChar = reader.ReadUInt8();
            encryptedChar ^= mask;
            encryptedChar ^= wz::WzKey[i];
            outputString.push_back((wchar_t) encryptedChar);
            mask++;
        }
    }
}

static void WzReadStringAtOffset(BinReader &reader, uint64 Offset, std::wstring &outputString) {
    uint64 position = reader.GetPosition();
    reader.SetPosition(Offset);
    WzReadString(reader, outputString);
    reader.SetPosition(position);
}

static void WzReadStringBlock(BinReader &reader, unsigned int offset, std::wstring &outputString) {
    switch (reader.ReadUInt8()) {
        case 0:
        case 0x73:
            WzReadString(reader, outputString);
            break;
        case 1:
        case 0x1B:
            WzReadStringAtOffset(reader, offset + reader.ReadUInt32(), outputString);
            break;
        default: {
            assert(0);
        }
    }
}

static int WzReadCompressedInt(BinReader &reader) {
    char ch = reader.ReadInt8();
    if (ch == -128) return reader.ReadInt32();
    return ch;
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

static unsigned int WzReadOffset(BinReader &reader, const WzFileDesc &fdesc) {
    unsigned int offset = (unsigned int) reader.GetPosition();
    offset = ~(offset - fdesc.m_Start);// ^ 0xFFFFFFFF;
    offset *= fdesc.m_Hash;
    offset -= wz::OffsetKey;
    offset = (offset << (offset & 0x1F)) | (offset >> (32 - (offset & 0x1F)));
    unsigned int encryptedOffset = reader.ReadUInt32();
    offset ^= encryptedOffset;
    offset += fdesc.m_Start * 2;
    return offset;
}

static bool WzIsImage(BinReader &reader) {
    unsigned char tcode = reader.ReadUInt8();

    if (tcode != 0x73) return false;
    else {
        std::wstring tstr;
        WzReadString(reader, tstr);
        if (tstr != L"Property") return false;
        else if (reader.ReadUInt16() != 0) return false;
    }

    return true;
}

static wz::WzCanvas WzParseCanvasProperty(BinReader &reader) {
    wz::WzCanvas canvas;
    canvas.m_Width = WzReadCompressedInt(reader);
    canvas.m_Height = WzReadCompressedInt(reader);
    canvas.m_Format = WzReadCompressedInt(reader);
    canvas.m_Format2 = reader.ReadUInt8();
    reader.ReadUInt32();
    canvas.m_Size = reader.ReadInt32() - 1;
    reader.ReadUInt8();

    canvas.m_Offset = reader.GetPosition();

    unsigned short header = reader.ReadUInt16();

    if (header != 0x9C78 && header != 0xDA78) {
        canvas.m_Encrypted = true;
    }

    switch (canvas.m_Format + canvas.m_Format2) {
        case 1: {
            canvas.m_UncompSize = canvas.m_Width * canvas.m_Height * 2;
        }
            break;
        case 2: {
            canvas.m_UncompSize = canvas.m_Width * canvas.m_Height * 4;
        }
            break;
        case 513:    // Format16bppRgb565
        {
            canvas.m_UncompSize = canvas.m_Width * canvas.m_Height * 2;
        }
            break;
        case 517: {
            canvas.m_UncompSize = canvas.m_Width * canvas.m_Height / 128;
        }
            break;
    }

    reader.SetPosition(canvas.m_Offset + canvas.m_Size);

    return canvas;
}

static void *WzDeflateCanvas(FileMapping &fm, const wz::WzCanvas &canvas) {
    if (canvas.m_Encrypted) {
        BinReader reader(fm);
        reader.SetPosition(canvas.m_Offset);
        uint64 endOfCanvas = canvas.m_Offset + canvas.m_Size;

        int blockSize = 0;
        std::vector<unsigned char> dataStream;

        while (reader.GetPosition() < endOfCanvas) {
            blockSize = reader.ReadInt32();
            for (int i = 0; i < blockSize; i++) {
                dataStream.push_back(reader.ReadUInt8() ^ wz::WzKey[i]);
            }
        }

        size_t dstLen = canvas.m_UncompSize;
        void *pBufDst = malloc(dstLen);
        uncompress((Bytef *) pBufDst, (uLongf *) &dstLen, (Bytef *) &dataStream[0], dataStream.size());
        return pBufDst;
    } else {
        size_t revlen = 0;
        void *pBufSrc = fm.Read(canvas.m_Offset, canvas.m_Size, &revlen);
        if (pBufSrc) {
            size_t dstLen = canvas.m_UncompSize;
            void *pBufDst = malloc(dstLen);
            uncompress((Bytef *) pBufDst, (uLongf *) &dstLen, (Bytef *) pBufSrc, revlen);
            return pBufDst;
        }
    }
    return 0;
}

void *WzRebuildCanvas(FileMapping &fm, const wz::WzCanvas &canvas, size_t &szBuf, size_t &bpp) {
    unsigned char *decBuf = (unsigned char *) WzDeflateCanvas(fm, canvas);
    unsigned char *argb = 0;
    if (decBuf) {
        switch (canvas.m_Format + canvas.m_Format2) {
            case 1: {
                int b, g;
                bpp = 32;
                szBuf = canvas.m_UncompSize * 2;
                argb = (unsigned char *) malloc(szBuf);
                for (int i = 0; i < canvas.m_UncompSize; i++) {
                    b = decBuf[i] & 0x0F;
                    b |= (b << 4);
                    argb[i * 2] = (unsigned char) b;

                    g = decBuf[i] & 0xF0;
                    g |= (g >> 4);
                    argb[i * 2 + 1] = (unsigned char) g;
                }
            }
                break;
            case 2:        // 32bpp ARGB8
            {
                bpp = 32;
                szBuf = canvas.m_UncompSize;

                argb = (unsigned char *) malloc(szBuf);
                memmove(argb, decBuf, szBuf);
            }
                break;
            case 513:    // 16bpp RGB565
            {
                bpp = 16;
                szBuf = canvas.m_UncompSize;

                argb = (unsigned char *) malloc(szBuf);
                memmove(argb, decBuf, szBuf);
            }
                break;
            case 517:    // ��ֵͼ
            {
                bpp = 8;
                szBuf = canvas.m_Width * canvas.m_Height;
                argb = (unsigned char *) malloc(szBuf);
                int x = 0, y = 0;
                unsigned char iB = 0;
                for (int i = 0; i < canvas.m_UncompSize; i++) {
                    for (unsigned char j = 0; j < 8; j++) {
                        iB = ((decBuf[i] & (0x01 << (7 - j))) >> (7 - j)) * 0xFF;
                        for (int k = 0; k < 16; k++) {

                            if (x == canvas.m_Width) {
                                x = 0;
                                y++;
                            }
                            argb[y * canvas.m_Width + x] = iB;
                            x++;
                        }
                    }
                }
            }
                break;
        }

        free(decBuf);
    }

    return argb;
}

static wz::WzSound WzParseSoundProperty(BinReader &reader) {
    wz::WzSound sound;
    reader.ReadUInt8();
    sound.m_Size = WzReadCompressedInt(reader);
    sound.m_TimeMS = WzReadCompressedInt(reader);
    reader.SetPosition(reader.GetPosition() + 56);
    sound.m_Frequency = reader.ReadInt32();
    reader.SetPosition(reader.GetPosition() + 22);

    sound.m_Offset = reader.GetPosition();

    reader.SetPosition(sound.m_Offset + sound.m_Size);

    return sound;
}

static bool WzParsePropertyList(BinReader &reader, unsigned int offset, wz::Node *root);

static void WzParseExtendedProp(BinReader &reader, unsigned int offset, const std::wstring &name, wz::Node *root) {
    /*
    std::wstring strPropName;
    WzReadStringBlock(reader, offset, strPropName);

    if (strPropName == L"Property") {
        auto* prop = new wz::Property<wz::WzSubProp>;
        reader.ReadUInt16();
        WzParsePropertyList(reader, offset, prop);
        root->appendChild(name, prop);
    } else if (strPropName == L"Canvas") {
        auto* canvas = new wz::Property<wz::WzCanvas>;
        reader.ReadUInt8();
        if (reader.ReadUInt8() == 1) {
            reader.ReadUInt16();
            WzParsePropertyList(reader, offset, canvas);
        }

        canvas->set(WzParseCanvasProperty(reader));

        root->appendChild(name, canvas);
    } else if (strPropName == L"Shape2D#Vector2D") {
        auto* vec = new wz::Property<wz::WzVec2D>;

        vec->set({
             WzReadCompressedInt(reader),
             WzReadCompressedInt(reader)
        });

        root->appendChild(name, vec);
    } else if (strPropName == L"Shape2D#Convex2D") {
        auto* shape = new wz::Property<wz::WzConvex>;

        int convexEntryCount = WzReadCompressedInt(reader);
        for (int i = 0; i < convexEntryCount; i++) {
            WzParseExtendedProp(reader, offset, name, shape);
        }

        root->appendChild(name, shape);
    } else if (strPropName == L"Sound_DX8") {
        auto* sound = new wz::Property<wz::WzSound>;
        sound->set(WzParseSoundProperty(reader));
        root->appendChild(name, sound);
    } else if (strPropName == L"UOL") {
        reader.ReadUInt8();
        auto* prop = new wz::Property<wz::WzUOL>;
        wz::WzUOL uol;
        WzReadStringBlock(reader, offset, uol.m_UOL);
        prop->set(uol);
        root->appendChild(name, prop);
    } else {
        assert(0);
    }*/
}

//////////////////////////////////////////////////////////////////////////
static bool WzParsePropertyList(BinReader &reader, unsigned int offset, wz::Node *root) {
    /*
    int entryCount = WzReadCompressedInt(reader);

    for (int i = 0; i < entryCount; i++) {
        std::wstring name;
        WzReadStringBlock(reader, offset, name);

        unsigned char utype = reader.ReadUInt8();
        switch (utype) {
            case 0: {
                auto *pProp = new wz::Property<WzNull>;
                root->appendChild(name, pProp);
            }
                break;
            case 0x0B:
            case 2: {
                auto* prop = new wz::Property<u16>(reader.ReadUInt16());
                root->appendChild(name, prop);
            }
                break;
            case 3: {
                auto* prop = new wz::Property<i32>(WzReadCompressedInt(reader));
                root->appendChild(name, prop);
            }
                break;
            case 4: {
                unsigned char type = reader.ReadUInt8();
                if (type == 0x80) {
                    auto* prop = new wz::Property<f32>(reader.ReadSingle());
                    root->appendChild(name, prop);
                } else if (type == 0) {
                    auto *pProp = new wz::Property<f32>(0.f);
                    root->appendChild(name, pProp);
                }
            }
                break;
            case 5: {
                auto* prop = new wz::Property<f64>(reader.ReadDouble());
                root->appendChild(name, prop);
            }
                break;
            case 8: {
                auto* prop = new wz::Property<std::wstring>;
                std::wstring str;
                WzReadStringBlock(reader, offset, str);
                prop->set(str);
                root->appendChild(name, prop);
            }
                break;
            case 9: {
                unsigned int ofs = reader.ReadUInt32();
                uint64 eob = reader.GetPosition() + ofs;
                WzParseExtendedProp(reader, offset, name, root);
                if (reader.GetPosition() != eob) reader.SetPosition(eob);
            }
                break;
            default: {
                assert(0);
                return false;
            }
        }
    }
*/
    return false;
}

static bool WzParseDirectory(BinReader &reader, const WzFileDesc &fdesc, wz::Node *root) {
    /*
    int entryCount = WzReadCompressedInt(reader);

    for (int i = 0; i < entryCount; i++) {
        unsigned char type = reader.ReadUInt8();
        std::wstring fname;
        int fsize = 0;
        int checksum = 0;
        unsigned int offset = 0;

        uint64 rememberPos = 0;

        if (type == 1) {
            int unknown = reader.ReadInt32();
            reader.ReadInt16();
            unsigned int offs = WzReadOffset(reader, fdesc);
            continue;
        } else if (type == 2) {
            int stringOffset = reader.ReadInt32();
            rememberPos = reader.GetPosition();
            reader.SetPosition(fdesc.m_Start + stringOffset);
            type = reader.ReadUInt8();
            WzReadString(reader, fname);
        } else if (type == 3 || type == 4) {
            WzReadString(reader, fname);
            rememberPos = reader.GetPosition();
        } else {
            assert(0);
        }

        reader.SetPosition(rememberPos);
        fsize = WzReadCompressedInt(reader);
        checksum = WzReadCompressedInt(reader);
        offset = WzReadOffset(reader, fdesc);

        if (!root && (offset >= reader.GetFileMapping().GetFileLen())) return false;

        if (type == 3) {
            if (root) {
                auto* new_dir = new wz::Directory(false, fsize, checksum, offset);
                root->appendChild(fname, new_dir);
            }
        } else {
            if (root) {
                auto* new_dir = new wz::Directory(true, fsize, checksum, offset);
                root->appendChild(fname, new_dir);
            } else {
                rememberPos = reader.GetPosition();

                reader.SetPosition(offset);
                if (!WzIsImage(reader)) return false;

                reader.SetPosition(rememberPos);
            }
        }
    }

    if (root) {
        for (const auto & it : *root) {
            for (auto cit : it.second) {
                wz::Node* pnode = cit;
                auto* pdir = dynamic_cast<wz::Directory*>(pnode);

                if (pdir) {
                    if (pdir->is_image()) {

                    } else {
                        reader.SetPosition(pdir->get_offset());
                        WzParseDirectory(reader, fdesc, pdir);
                    }
                }
            }
        }
    }*/

    return false;
}

//////////////////////////////////////////////////////////////////////////
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

void WzParseListFile(BinReader &reader, std::vector<std::wstring> &strList) {
    while (!reader.IsEOF()) {
        int len = reader.ReadInt32();
        std::wstring str, dstr;
        str.resize(len);
        dstr.resize(len);

        for (int i = 0; i < len; i++) {
            str[i] = reader.ReadUInt16();
        }

        reader.ReadUInt16();

        WzDecryptString(str, dstr);
        strList.push_back(dstr);
    }
}

bool WzParseFile(BinReader &reader, WzFileDesc &fdesc, wz::Node *root) {
    unsigned int hdr = reader.ReadUInt32();
    if (hdr != wz::HeaderMagic) return false;
    uint64 fsize = reader.ReadUInt64();
    unsigned int fstart = reader.ReadUInt32();

    std::string copyright;
    reader.ReadAstring(copyright);

    reader.SetPosition(fstart);

    short encVersion = reader.ReadInt16();

    short fileVersion = -1;
    unsigned int versionHash = 0;

    for (int j = 0; j < 0x7FFF; j++) {
        fileVersion = (short) j;
        versionHash = WzGetVersionHash(encVersion, fileVersion);
        if (versionHash != 0) {
            fdesc.m_Start = fstart;
            fdesc.m_Hash = versionHash;
            fdesc.m_FileVersion = fileVersion;

            uint64 position = reader.GetPosition();
            if (!WzParseDirectory(reader, fdesc, 0)) {
                reader.SetPosition(position);
                continue;
            } else {
                if (root) {
                    reader.SetPosition(position);
                    WzParseDirectory(reader, fdesc, root);
                }
                return true;
            }
        }
    }

    return false;
}

bool WzParseImage(BinReader &reader, const wz::Directory *pimg, wz::Node *root) {
    if (pimg && pimg->is_image()) {
        reader.SetPosition(pimg->get_offset());
        if (WzIsImage(reader)) {
            return WzParsePropertyList(reader, pimg->get_offset(), root);
        }
    }
    return false;
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

