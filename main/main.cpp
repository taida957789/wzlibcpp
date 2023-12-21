#include <iostream>


#include <algorithm>

#include <wz/Node.hpp>
#include <wz/File.hpp>
#include <wz/Directory.hpp>
#include <wz/Property.hpp>

#include <iostream>
#include <fstream> // 包含文件操作的头文件

#define U8 static_cast<u8>
#define IV4(A, B, C, D)            \
    {                              \
        U8(A), U8(B), U8(C), U8(D) \
    }

int main()
{
    const auto iv = IV4(0xb9, 0x7d, 0x63, 0xe9);
    wz::File file(iv, "C:/Users/Shocker/Desktop/gdtest/Data/Map.wz");

    if (file.parse())
    {
        wz::Node *node=file.get_root()->find_from_path(u"Map/Map1/101000000.img");
        // wz::Node *node = wz::Node::find_from_path(file.get_root(), );
        auto layer=node->children[u"0"];
        auto canvans = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
        std::ofstream outfile("./output.rgb", ios::binary);
        auto buf = canvans->get_raw_data();
        int size = buf.size();
        outfile.write((char *)buf.data(), size);
        outfile.close();
        return 1;
    }
    _sleep(90000000);
    return 0;
}