#include <cassert>
#include "Node.hpp"
#include "Property.hpp"

wz::Node::Node()
        : parent(nullptr), reader(nullptr), type(Type::NotSet) {
}

wz::Node::Node(const Type& new_type, Reader& from_file)
        : parent(nullptr), reader(& from_file), type(new_type) {
}

wz::Node::~Node() {
    for (auto&[_, nodes] : children) {
        for (auto* node : nodes) {
            delete node;
        }
    }
}

void wz::Node::appendChild(const std::wstring& name, Node* node) {
    assert(node);
    children[name].push_back(node);
    node->parent = this;
}

const wz::WzMap& wz::Node::get_children() const {
    return children;
}

wz::Node* wz::Node::get_parent() const {
    return parent;
}

wz::WzMap::iterator wz::Node::begin() {
    return children.begin();
}

wz::WzMap::iterator wz::Node::end() {
    return children.end();
}

size_t wz::Node::children_count() const {
    return children.size();
}

bool wz::Node::parse_property_list(Node* target, size_t offset) {
    auto entryCount = reader->read_compressed_int();

    for (i32 i = 0; i < entryCount; i++) {
        auto name = reader->read_string_block(offset);

        auto prop_type = reader->read<u8>();
        switch (prop_type) {
            case 0: {
                auto* prop = new wz::Property<WzNull>(Type::Null, *reader);
                target->appendChild(name, prop);
            }
                break;
            case 0x0B:
                [[fallthrough]];
            case 2: {
                auto* prop = new wz::Property<u16>(Type::UnsignedShort, *reader, reader->read<u16>());
                target->appendChild(name, prop);
            }
                break;
            case 3: {
                auto* prop = new wz::Property<i32>(Type::Int, *reader, reader->read_compressed_int());
                target->appendChild(name, prop);
            }
                break;
            case 4: {
                auto float_type = reader->read<u8>();
                if (float_type == 0x80) {
                    auto* prop = new wz::Property<f32>(Type::Float, *reader, reader->read<f32>());
                    target->appendChild(name, prop);
                } else if (float_type == 0) {
                    auto* pProp = new wz::Property<f32>(Type::Float, *reader, 0.f);
                    target->appendChild(name, pProp);
                }
            }
                break;
            case 5: {
                auto* prop = new wz::Property<f64>(Type::Double, *reader, reader->read<f64>());
                target->appendChild(name, prop);
            }
                break;
            case 8: {
                auto* prop = new wz::Property<std::wstring>(Type::String, *reader);
                auto str = reader->read_string_block(offset);
                prop->set(str);
                target->appendChild(name, prop);
            }
                break;
            case 9: {
                auto ofs = reader->read<u32>();
                auto eob = reader->get_position() + ofs;
                parse_extended_prop(name, target, offset);
                if (reader->get_position() != eob) reader->set_position(eob);
            }
                break;
            default: {
                assert(0);
            }
        }
    }

    return true;
}

void wz::Node::parse_extended_prop(const std::wstring& name, wz::Node* target, const size_t& offset) {
    auto strPropName = reader->read_string_block(offset);

    if (strPropName == L"Property") {
        auto* prop = new Property<WzSubProp>(Type::SubProperty, *reader);
        reader->skip(sizeof(u16));
        parse_property_list(prop, offset);
        target->appendChild(name, prop);
    } else if (strPropName == L"Canvas") {
        auto* prop = new Property<WzCanvas>(Type::Canvas, *reader);
        reader->skip(sizeof(u8));
        if (reader->read<u8>() == 1) {
            reader->skip(sizeof(u16));
            parse_property_list(prop, offset);
        }

        prop->set(parse_canvas_property());

        target->appendChild(name, prop);
    } else if (strPropName == L"Shape2D#Vector2D") {
        auto* prop = new Property<WzVec2D>(Type::Vector2D, *reader);

        auto x = reader->read_compressed_int();
        auto y = reader->read_compressed_int();
        prop->set({x, y});

        target->appendChild(name, prop);
    } else if (strPropName == L"Shape2D#Convex2D") {
        auto* prop = new Property<WzConvex>(Type::Convex2D, *reader);

        int convexEntryCount = reader->read_compressed_int();
        for (int i = 0; i < convexEntryCount; i++) {
            parse_extended_prop(name, prop, offset);
        }

        target->appendChild(name, prop);
    } else if (strPropName == L"Sound_DX8") {
        auto* prop = new Property<WzSound>(Type::Sound, *reader);

        prop->set(parse_sound_property());

        target->appendChild(name, prop);
    } else if (strPropName == L"UOL") {
        reader->skip(sizeof(u8));
        auto* prop = new Property<WzUOL>(Type::UOL, *reader);
        prop->set({ reader->read_string_block(offset) });
        target->appendChild(name, prop);
    } else {
        assert(0);
    }

}

wz::WzCanvas wz::Node::parse_canvas_property() {
    WzCanvas canvas;
    canvas.m_Width = reader->read_compressed_int();
    canvas.m_Height = reader->read_compressed_int();
    canvas.m_Format = reader->read_compressed_int();
    canvas.m_Format2 = reader->read<u8>();
    reader->skip(sizeof(u32));
    canvas.m_Size = reader->read<i32>() - 1;
    reader->skip(sizeof(u8));

    canvas.m_Offset = reader->get_position();

    auto header = reader->read<u16>();

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

    reader->set_position(canvas.m_Offset + canvas.m_Size);

    return canvas;
}

wz::WzSound wz::Node::parse_sound_property() {
    WzSound sound;
    // reader->ReadUInt8();
    reader->skip(sizeof(u8));
    sound.m_Size = reader->read_compressed_int();
    sound.m_TimeMS = reader->read_compressed_int();
    reader->set_position(reader->get_position() + 56);
    sound.m_Frequency = reader->read<i32>();
    reader->set_position(reader->get_position() + 22);

    sound.m_Offset = reader->get_position();

    reader->set_position(sound.m_Offset + sound.m_Size);

    return sound;
}

wz::Type wz::Node::get_type() const {
    return type;
}

bool wz::Node::is_property() const {
    return (bit(type) & bit(Type::Property)) == bit(Type::Property);
}
