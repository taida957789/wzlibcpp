#pragma once

#include "Node.hpp"

namespace wz {
    template<typename T>
    class Property : public Node {
    public:
        explicit Property(Reader& from_file) : Node(Type::Property, from_file) {}

        explicit Property(Reader& from_file, T new_data)
            : data(new_data), Node(Type::Property, from_file) {
        };

        void set(T new_data) {
            data = new_data;
        }

        [[maybe_unused]]
        const T& get() const {
            return data;
        }

    private:
        T data;
    };
}
