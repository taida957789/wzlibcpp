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
constexpr auto strings(T iterable)
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

wz::WzMap find_from_path(wz::Node *root, const std::u16string &path)
{
    std::vector<std::string> next{};
    std::string s{};
    s.assign(path.begin(), path.end());
    pystring::split(s, next, "/");
    std::reverse(next.begin(), next.end());
    const auto current = strings(pop(next));
    for (const auto &it : *root)
    {
        if (it.first == current)
        {
            if (next.empty())
            {
                return root->get_children();
            }

            std::reverse(next.begin(), next.end());
            return find_from_path(root, strings(pystring::join("/", next)));
        }
    }
    return {};
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
        wz::WzMap node = find_from_path(file.get_root(), u"00002000.img");
        for (const auto &it : node)
        {
            // for (const auto &c : it.first)
            // {
            //     std::cout << static_cast<char>(c);
            // }
            // std::cout<<std::endl;
            // std::wcout << it.first << ", " << it.second.size() << std::endl;
            auto *dir = dynamic_cast<wz::Directory *>(it.second[0]);
            if (dir && dir->is_image())
            {
                auto *image = new wz::Node();
                dir->parse_image(image);

                for (const auto &n : *image)
                {
                    if (n.first == u"alert")
                    {
                        /* code */
                        auto aa = n.second[0]->get_children();
                        auto a = aa.at(u"0");
                        // wz::Property p(wz::WzCanvas,file,a);
                        // dir->parse_canvas_property();
                        // wz::Property b=wz::Property(a.at(0)->get_child(u"arm"));
                        auto c = a.at(0)->get_child(u"arm");
                        auto d = dynamic_cast<wz::Property<wz::WzCanvas> *>(c);
                        std::ofstream outfile("./output.bmp", ios::binary);
                        auto buf = d->get_png();
                        int size = buf.size();
                        outfile.write((char *)buf.data(), size);
                        std::cout << std::endl;
                        outfile.close();
                        return 1;
                    }

                    for (const auto &c : n.first)
                    {
                        std::cout << static_cast<char>(c);
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    else
    {
        printf("fail");
    }
    _sleep(90000000);
    return 0;
}