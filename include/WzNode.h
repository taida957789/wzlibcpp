#ifndef __WzNode_h__
#define __WzNode_h__

#include <map>
#include <vector>
#include <string>
#include "NumTypes.hpp"

class WzNode;

typedef std::vector<WzNode *> WzNodeList;
typedef std::map<std::wstring, WzNodeList> WzNodeMap;

class WzNode {
public:
    WzNode();

    virtual void AddChild(const std::wstring &name, WzNode *node);

    virtual const WzNodeMap& GetChilds() const;

    // todo: deprecate
    virtual void Delete();

    // todo: deprecate
    virtual void Free();

    // todo: deprecate
    virtual bool FreeChild(const std::wstring &name);

    // todo: deprecate
    virtual void FreeChilds();

    virtual WzNode *GetParent();

    [[maybe_unused]]
    WzNodeMap::iterator begin() {
        return children.begin();
    }

    [[maybe_unused]]
    WzNodeMap::iterator end() {
        return children.end();
    }

public:
    static WzNode *New();

protected:
    virtual ~WzNode();

protected:
    WzNode* parent;
    WzNodeMap children;
};

//////////////////////////////////////////////////////////////////////////
class WzDirectory : public WzNode {
public:
    WzDirectory();

    void Set(bool img, int size, int checksum, unsigned int offset);

    [[nodiscard]]
    u32 GetOffset() const;

    [[nodiscard]]
    bool IsImage() const;

public:
    static WzDirectory *New();

private:
    bool image;
    [[maybe_unused]]
    int size;
    [[maybe_unused]]
    int checksum;
    unsigned int offset;
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
class WzProperty : public WzNode {
public:
    WzProperty() {}

    void Set(T data) {
        m_Data = data;
    }

    const T &Get() const {
        return m_Data;
    }

public:
    static WzProperty<T> *New() {
        return new WzProperty<T>();
    }

private:
    T m_Data;
};

#endif