#pragma once

#include "Node.h"

namespace wz {
    template<typename T>
    class Property : public Node {
    public:
        explicit Property() = default;

        explicit Property(T new_data)
            : data(new_data) {
        };

        [[deprecated]]
        void Set(T new_data) {
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
