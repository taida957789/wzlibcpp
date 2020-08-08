#pragma once

#include <map>
#include <vector>
#include <string>

#include "Reader.hpp"
#include "Types.hpp"

namespace wz {

    class Node;

    typedef std::vector<Node*> WzList;
    typedef std::map<std::wstring, WzList> WzMap;

    class Node {
    public:

        explicit Node();
        explicit Node(Reader& from_file);

        virtual void appendChild(const std::wstring& name, Node* node);

        [[nodiscard]]
        virtual const WzMap& get_children() const;

        [[deprecated]]
        virtual void Delete();

        [[deprecated]]
        virtual void Free();

        [[deprecated]]
        virtual bool FreeChild(const std::wstring& name);

        [[deprecated]]
        virtual void FreeChilds();

        [[nodiscard]]
        virtual Node* getParent() const;

        [[nodiscard]] [[maybe_unused]]
        auto children_count() const;

        [[maybe_unused]]
        WzMap::iterator begin();

        [[maybe_unused]]
        WzMap::iterator end();

    protected:
        virtual ~Node();

        Node* parent;
        WzMap children;

        Reader* reader;

        bool parsePropertyList(Node* target, size_t offset);
        void parseExtendProp(const std::wstring& name, Node* target, const size_t& offset);
        WzCanvas parseCanvasProperty();
        WzSound parseSoundProperty();

        friend class Directory;

    };

}