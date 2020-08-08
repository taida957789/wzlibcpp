#pragma once

#include "Node.h"

namespace wz {
    template<typename T>
    class Property : public Node {
    public:
        explicit Property(Reader& from_file) : Node(from_file) {}

        explicit Property(Reader& from_file, T new_data)
            : data(new_data), Node(from_file) {
        };

        void set(T new_data) {
            data = new_data;
        }

        const T& get() const {
            return data;
        }

    public:
        [[deprecated]]
        static Property<T> *New() {
            return new Property<T>();
        }

    private:
        T data;
    };
}
