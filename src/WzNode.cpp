#include <assert.h>
#include "WzNode.h"

//////////////////////////////////////////////////////////////////////////
WzNode::WzNode() : parent(0) {}

WzNode::~WzNode() {}

void WzNode::AddChild(const std::wstring &name, WzNode *node) {
    assert(node);
    children[name].push_back(node);
    node->parent = this;
}

const WzNodeMap &WzNode::GetChilds() const {
    return children;
}

void WzNode::Delete() {
    delete this;
}

void WzNode::Free() {
    for (WzNodeMap::iterator it = children.begin();
         it != children.end();
         it++) {
        for (WzNodeList::iterator cit = it->second.begin();
             cit != it->second.end();
             cit++) {
            (*cit)->Free();
        }
    }

    Delete();
}

bool WzNode::FreeChild(const std::wstring &name) {
    WzNodeMap::iterator it = children.find(name);
    if (it != children.end()) {
        for (WzNodeList::iterator cit = it->second.begin();
             cit != it->second.end();
             cit++) {
            (*cit)->Free();
        }

        children.erase(it);
        return true;
    }
    return false;
}

void WzNode::FreeChilds() {
    for (WzNodeMap::iterator it = children.begin();
         it != children.end();
         it++) {
        for (WzNodeList::iterator cit = it->second.begin();
             cit != it->second.end();
             cit++) {
            (*cit)->Free();
        }
    }
    children.clear();
}

WzNode *WzNode::GetParent() {
    return parent;
}

WzNode *WzNode::New() {
    return new WzNode();
}

//////////////////////////////////////////////////////////////////////////
WzDirectory::WzDirectory() : image(false), size(0), checksum(0), offset(0) {}

void WzDirectory::Set(bool img, int size, int checksum, unsigned int offset) {
    image = img;
    size = size;
    checksum = checksum;
    offset = offset;
}

unsigned int WzDirectory::GetOffset() const {
    return offset;
}

bool WzDirectory::IsImage() const {
    return image;
}

WzDirectory *WzDirectory::New() {
    return new WzDirectory();
}