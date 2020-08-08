#pragma once

#include <string>
#include "NumTypes.hpp"

namespace wz {

    struct WzNull {

    };

    struct WzSubProp {

    };

    struct WzConvex {

    };

    struct WzUOL {
        std::wstring m_UOL;
    };

    struct WzCanvas {
        i32 m_Width;
        i32 m_Height;
        i32 m_Format;
        i32 m_Format2;
        bool m_Encrypted;
        i32 m_Size;
        i32 m_UncompSize;
        size_t m_Offset;

        WzCanvas()
                : m_Width(0), m_Height(0),
                  m_Format(0), m_Format2(0),
                  m_Encrypted(false), m_Size(0), m_UncompSize(0),
                  m_Offset(0) {}
    };

    struct WzSound {
        i32 m_TimeMS;
        i32 m_Frequency;
        i32 m_Size;
        size_t m_Offset;

        WzSound() : m_TimeMS(0), m_Frequency(0), m_Size(0), m_Offset(0) {}
    };

    struct WzVec2D {

        int m_X;
        int m_Y;

        WzVec2D() : m_X(0), m_Y(0) {}
        WzVec2D(int x, int y) : m_X(x), m_Y(y) {}
    };
}