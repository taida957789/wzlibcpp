#pragma once

#include "Node.hpp"

namespace wz {
    template<typename T>
    class Property : public Node {
    public:
        explicit Property(const Type& new_type, Reader& from_file) : Node(new_type, from_file) {
            assert(is_property());
        }

        explicit Property(const Type& new_type, Reader& from_file, T new_data)
            : data(new_data), Node(new_type, from_file) {
            assert(is_property());
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
