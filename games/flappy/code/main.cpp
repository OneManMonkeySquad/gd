
import <windows.h>;
import foundation;
import std;

int main()
{
    foundation::install_global_exception_handler();

    foundation::clear_log();
    foundation::println("*-*-* INIT *-*-*");

#ifdef _WIN32
    std::setlocale(2, ".UTF8");
#endif

    foundation::api_registry api_registry;

    foundation::plugin_manager plugin_manager{ api_registry };
    plugin_manager.init();

    //
    foundation::println("*-*-* RUN *-*-*");

    {
        auto game = (foundation::IGame*)api_registry.first("game");
        game->start();
    }

    while (true)
    {
        auto game = (foundation::IGame*)api_registry.first("game");
        if (!game->run_once())
            break;

        plugin_manager.update();
    }

    foundation::println("*-*-* EXIT *-*-*");

    {
        auto game = (foundation::IGame*)api_registry.first("game");
        game->exit();
    }

    plugin_manager.exit();

    return 0;
}