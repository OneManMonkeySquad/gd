#pragma once

#include "api.h"
#include <Windows.h>
#include <list>
#include <entt/entt.hpp>

//import std;
//import <entt/entt.hpp>;

namespace fd
{
    struct api_registry_t
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


        template<typename T>
        void reset()
        {
            set<T>(T{});
        }

        /// <summary>
        /// Get api pointer. May return a valid pointer with INVALID function
        /// pointers that will be patched later when the corresponding api is set.
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

    API api_registry_t* get_api_registry();
}