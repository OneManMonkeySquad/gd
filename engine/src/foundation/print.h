#pragma once

#include "api.h"
#include <Windows.h>
#include <format>
#include <fstream>

namespace fd
{
    API void clear_log();

    API void println(const char* str);

    template<typename... Args>
    void println(std::format_string<Args...> fmt, Args&&... args)
    {
        auto str = std::format(fmt, std::forward<Args>(args)...);
        str += '\n';
        ::OutputDebugStringA(str.c_str());

        std::ofstream outfile;
        outfile.open("game.log", std::ios_base::app);
        outfile << str;
    }
}