#include "SDL3\SDL_filesystem.h"

import <windows.h>;
import foundation;
import std;

namespace fs = std::filesystem;

namespace
{
    fd::api_registry_t api_registry;
}

void run()
{
    fd::install_global_exception_handler();

    fd::clear_log();

    fd::println("");
    fd::println("*-*-* INIT *-*-*");

#ifdef _WIN32
    std::setlocale(2, ".UTF8");
#endif

    auto game = api_registry.get<fd::game_t>();
    auto platform = api_registry.get<fd::platform_t>();

    fd::plugin_manager_t plugin_manager{ api_registry };
    plugin_manager.init();

    fd::println("");
    fd::println("*-*-* RUN *-*-*");

    auto js = api_registry.get<fd::job_system_t>();
    js->init();

    platform->init();

    game->start();

    bool once = false;
    while (true)
    {
        if (!game->run_once())
            break;

        plugin_manager.update();
        if (!once)
        {
            once = true;
            plugin_manager.dirty_file_manually(fs::path{ SDL_GetBasePath() } / "plugin_game.dll");
        }
    }

    fd::println("");
    fd::println("*-*-* EXIT *-*-*");

    game->exit();

    platform->exit();

    js->deinit();

    plugin_manager.exit();
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        fd::println(e.what());
    }
    return 0;
}