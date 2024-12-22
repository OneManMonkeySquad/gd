#include "api_registry.h"

namespace fd
{
    api_registry_t* get_api_registry()
    {
        static api_registry_t global;
        return &global;
    }
}