#include "print.h"

#include "SDL3\SDL_filesystem.h"

namespace fd
{
    FILE* log_file = nullptr;

    void clear_log()
    {
        std::string path = SDL_GetBasePath();
        path += "/game.log";
        log_file = fopen(path.c_str(), "w");

        // Disable buffering
        // todo slow
        setvbuf(log_file, NULL, _IONBF, 0);
    }

    void println(const char* str)
    {
        std::string foo = str;
        foo += '\n';

        ::OutputDebugStringA(foo.c_str());
        fputs(foo.c_str(), log_file);
    }
}