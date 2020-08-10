#pragma once

#include <map>
#include <vector>
#include <string>

#include "Wz.hpp"
#include "Reader.hpp"
#include "Types.hpp"

namespace wz {

    class Node;

    typedef std::vector<Node*> WzList;
    typedef std::map<std::wstring, WzList> WzMap;

    class Node {
    public:

        explicit Node();
        explicit Node(const Type& new_type, Reader& from_file);

        virtual ~Node();

        virtual void appendChild(const std::wstring& name, Node* node);

        [[maybe_unused]] [[nodiscard]]
        virtual const WzMap& get_children() const;

        [[maybe_unused]] [[nodiscard]]
        virtual Node* get_parent() const;

        [[nodiscard]] [[maybe_unused]]
        size_t children_count() const;

        WzMap::iterator begin();

        WzMap::iterator end();

        [[maybe_unused]] [[nodiscard]]
        Type get_type() const;

    protected:

        Type type;

        Node* parent;
        WzMap children;

        Reader* reader;

        bool parse_property_list(Node* target, size_t offset);
        void parse_extended_prop(const std::wstring& name, Node* target, const size_t& offset);
        WzCanvas parse_canvas_property();
        WzSound parse_sound_property();

        friend class Directory;

    };

}