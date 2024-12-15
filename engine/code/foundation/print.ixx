export module foundation:print;

import std;
import <Windows.h>;

namespace foundation
{
    export void clear_log()
    {
        std::ofstream outfile;
        outfile.open("game.log", std::ios_base::trunc);
    }

    export template<typename... Args>
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