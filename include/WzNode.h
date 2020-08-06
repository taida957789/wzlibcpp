#ifndef __WzNode_h__
#define __WzNode_h__

#include <map>
#include <vector>
#include <string>

class WzNode;

typedef std::vector<WzNode*> WzNodeList;
typedef std::map<std::wstring, WzNodeList> WzNodeMap;

class WzNode {
public:
    WzNode();

    virtual void AddChild(const std::wstring& name, WzNode* node);

    virtual const WzNodeMap& GetChilds() const;

    virtual void Delete();

    virtual void Free();

    virtual bool FreeChild(const std::wstring& name);

    virtual void FreeChilds();

    virtual WzNode* GetParent();

    WzNodeMap::iterator begin() {
        return children.begin();
    }

    WzNodeMap::iterator end() {
        return children.end();
    }

public:
    static WzNode* New();

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

    void Set(bool img, int new_size, int new_checksum, unsigned int new_offset);

    unsigned int GetOffset() const;

    bool IsImage() const;

public:
    static WzDirectory* New();

private:
    bool image;
    int size;
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

    const T& Get() const {
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