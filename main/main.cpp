#include <iostream>

#include <pystring.h>
#include <algorithm>

#include <wz/Wz.hpp>
#include <wz/Node.hpp>
#include <wz/File.hpp>
#include <wz/Directory.hpp>
#include <wz/Property.hpp>

#include <iostream>
#include <fstream> // 包含文件操作的头文件
#include "Keys.hpp"

template <typename T>
constexpr auto ustring(T iterable)
{
    return std::u16string{iterable.begin(), iterable.end()};
}

template <typename T>
constexpr auto back(T iterable)
{
    return std::string{iterable.begin(), iterable.end()};
}

template <typename T>
T pop(std::vector<T> &vec)
{
    auto last = vec.back();
    vec.pop_back();
    return last;
}

wz::Node *find_from_path(wz::Node *root, const std::u16string &path)
{
    std::vector<std::string> next{};
    std::string s{};
    s.assign(path.begin(), path.end());
    pystring::split(s, next, "/");
    wz::Node *node = root;
    for (auto s : next)
    {
        for (const auto &it : *node)
        {
            if (it.first == ustring(s))
            {
                auto *dir = dynamic_cast<wz::Directory *>(it.second[0]);
                if (dir->is_image())
                {
                    auto *image = new wz::Node();
                    dir->parse_image(image);
                    node = image;
                    break;
                }
                else
                {
                    node = it.second[0];
                    break;
                }
            }
        }
    }
    return node;
}

#define U8 static_cast<u8>
#define IV4(A, B, C, D)            \
    {                              \
        U8(A), U8(B), U8(C), U8(D) \
    }

int main()
{
    const auto iv = IV4(0xb9, 0x7d, 0x63, 0xe9);
    wz::File file(iv, "C:/Users/Shocker/Desktop/StudyMS/Data/Character.wz");

    if (file.parse())
    {
        wz::Node *node = find_from_path(file.get_root(), u"00012000.img");
        for (const auto &it : *node)
        {
            if (it.first == u"front")
            {
                /* code */
                auto aa = it.second[0]->get_children();
                auto a = aa.at(u"head");
                auto c = a.at(0);
                auto d = dynamic_cast<wz::Property<wz::WzCanvas> *>(c);
                std::ofstream outfile("./output.rgb", ios::binary);
                auto buf = d->get_png();
                int size = buf.size();
                outfile.write((char *)buf.data(), size);
                std::cout << std::endl;
                outfile.close();
                return 1;
            }
        }
    }
    _sleep(90000000);
    return 0;
}