module;

#include <Windows.h>

export module foundation:api_registry;

import std;

namespace foundation
{
    export struct api_registry
    {
        std::vector<std::pair<std::string, void*>> foo;

        void add(std::string name, void* api)
        {
            foo.push_back(std::make_pair(name, api));
        }

        void remove(void* api)
        {
            for (auto it = std::begin(foo); it != std::end(foo); ++it)
            {
                if (it->second == api)
                {
                    foo.erase(it);
                    break;
                }
            }
        }

        void* first(std::string name)
        {
            for (auto& pair : foo)
            {
                if (pair.first == name)
                    return pair.second;
            }
            return nullptr;
        }
    };
}