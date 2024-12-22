#include "print.h"

namespace fd
{
    void clear_log()
    {
        std::ofstream outfile;
        outfile.open("game.log", std::ios_base::trunc);
    }

    void println(const char* str)
    {
        ::OutputDebugStringA(str);

        std::ofstream outfile;
        outfile.open("game.log", std::ios_base::app);
        outfile << str;
    }
}