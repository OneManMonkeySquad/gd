
import <windows.h>;
import foundation;
import std;

int main()
{
    foundation::install_global_exception_handler();

    foundation::clear_log();

    foundation::println("");
    foundation::println("*-*-* INIT *-*-*");

#ifdef _WIN32
    std::setlocale(2, ".UTF8");
#endif

    foundation::api_registry api_registry;

    auto game = api_registry.get<foundation::IGame>();

    foundation::plugin_manager plugin_manager{ api_registry };
    plugin_manager.init();

    foundation::println("");
    foundation::println("*-*-* RUN *-*-*");

    game->start();

    while (true)
    {
        if (!game->run_once())
            break;

        plugin_manager.update();
    }

    foundation::println("");
    foundation::println("*-*-* EXIT *-*-*");

    game->exit();

    plugin_manager.exit();

    return 0;
}