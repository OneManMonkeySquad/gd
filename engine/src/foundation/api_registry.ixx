module;

#include <Windows.h>

export module foundation:api_registry_t;

import std;
import <entt/entt.hpp>;
import :print;

namespace fd
{
    export struct api_registry_t
    {
        // note: this needs to be a list for api pointers to have a fixed address
        std::list<std::pair<entt::id_type, void*>> _foo;

        template<typename T>
        void set(const T& api)
        {
            auto name = entt::type_id<T>().hash();

            for (auto& pair : _foo)
            {
                if (pair.first == name)
                {
                    memcpy(pair.second, &api, sizeof(T));
                    return;
                }
            }

            _foo.push_back(std::make_pair(name, new T(api)));
        }

        void remove(void* api)
        {
            for (auto& pair : _foo)
            {
                if (pair.second == api)
                {
                    pair.second = nullptr;
                    return;
                }
            }
        }

        /// <summary>
        /// Get api pointer. May return an invalid pointer that is
        /// patched later when the corresponding api is loaded.
        /// </summary>
        template<typename T>
        T* get()
        {
            auto name = entt::type_id<T>().hash();

            for (auto& pair : _foo)
            {
                if (pair.first == name)
                    return (T*)pair.second;
            }

            auto& ref = _foo.emplace_back(name, new T());
            return (T*)ref.second;
        }
    };
}