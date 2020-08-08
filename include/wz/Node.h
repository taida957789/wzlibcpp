#pragma once

#include <map>
#include <vector>
#include <string>

namespace wz {

    class Node;

    typedef std::vector<Node*> WzList;
    typedef std::map<std::wstring, WzList> WzMap;

    class Node {
    public:
        Node();

        virtual void appendChild(const std::wstring& name, Node* node);

        [[deprecated]]
        virtual const WzMap& GetChilds() const;

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

    public:
        static Node* New();

    protected:
        virtual ~Node();

    protected:
        Node* parent;
        WzMap children;
    };

}