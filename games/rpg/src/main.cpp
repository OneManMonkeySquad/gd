#include "foundation\plugin_manager.h"
#include "foundation\print.h"
#include "foundation\api_registry.h"
#include "foundation\foundation.h"
#include "SDL3\SDL_filesystem.h"
#include "rpg_game/rpg_game.h"
#include <Windows.h>

namespace fs = std::filesystem;

namespace
{
    fd::plugin_manager_t plugin_manager;
}


void run()
{
    // fd::install_global_exception_handler();

    fd::clear_log();

    fd::println("");
    fd::println("*-*-* INIT *-*-*");

#ifdef _WIN32
    std::setlocale(2, ".UTF8");
#endif

    auto api_registry = fd::get_api_registry();
    auto game = api_registry->get<rpg_game_t>();
    auto platform = api_registry->get<fd::platform_t>();

    plugin_manager.init();

    fd::println("");
    fd::println("*-*-* RUN *-*-*");

    auto js = api_registry->get<fd::job_system_t>();
    js->init();

    platform->init();

    game->start();

    while (true)
    {

        platform->update();

        try
        {
            if (game->run_once() != update_result::keep_running)
                break;
        }
        catch (const std::exception&)
        {
            fd::println("crash");
            plugin_manager.blocking_wait_until_any_plugin_changes();
        }

        plugin_manager.update();
    }

    fd::println("");
    fd::println("*-*-* EXIT *-*-*");

    game->exit();

    platform->exit();

    js->deinit();

    plugin_manager.unload_all_plugins();
}

void foo(unsigned int, struct _EXCEPTION_POINTERS*)
{
    throw std::runtime_error("foo");
}

int main()
{
    _set_se_translator(foo);

    run();
    return 0;
}