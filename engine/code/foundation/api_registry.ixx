module;

#include <Windows.h>

export module foundation:api_registry;

import std;
import <entt/entt.hpp>;
import :print;

namespace foundation
{
    export template<typename T>
        class api
    {
    public:
        api(T** ptr) : _ptr(ptr) {}

        [[nodiscard]] constexpr T* raw_ptr() noexcept
        {
            return *_ptr;
        }

        [[nodiscard]] constexpr T* operator->() noexcept
        {
            return *_ptr;
        }

    private:
        T** _ptr;
    };

    export struct api_registry
    {
        // note: this needs to be a list for api pointers to have a fixed address
        std::list<std::pair<entt::id_type, void*>> _foo;

        template<typename T>
        void set(void* api)
        {
            auto name = entt::type_id<T>().hash();

            for (auto& pair : _foo)
            {
                if (pair.first == name)
                {
                    foundation::println("patched existing pointer");

                    pair.second = api;
                    return;
                }
            }

            _foo.push_back(std::make_pair(name, api));
        }

        void remove(void* api)
        {
            for (auto it = std::begin(_foo); it != std::end(_foo); ++it)
            {
                if (it->second == api)
                {
                    _foo.erase(it);
                    break;
                }
            }
        }

        /// <summary>
        /// Get api pointer. May return an invalid pointer that gets
        /// patched later when the corresponding api is loaded.
        /// </summary>
        template<typename T>
        api<T> get()
        {
            auto name = entt::type_id<T>().hash();

            for (auto& pair : _foo)
            {
                if (pair.first == name)
                    return (T**)&pair.second;
            }

            foundation::println("created dummy pointer");
            auto& ref = _foo.emplace_back(name, nullptr);
            return (T**)&ref.second;
        }
    };
}