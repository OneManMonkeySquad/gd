import <SDL3\SDL_filesystem.h>;
import <windows.h>;
import foundation;
import std;

namespace fs = std::filesystem;

using plugin_load_type = void(*)(foundation::api_registry&);

int main()
{
    foundation::println("*-*-* Startup *-*-*");

    foundation::api_registry api_registry;

    // Load plugins
    for (const auto& dir_entry : fs::directory_iterator{ SDL_GetBasePath() })
    {
        const auto filename = dir_entry.path().filename().string();
        bool is_plugin = filename.starts_with("plugin_") && filename.ends_with(".dll");
        if (is_plugin)
        {
            foundation::println("Loading plugin '{}'...", dir_entry.path().string());

            auto plugin = ::LoadLibraryW(dir_entry.path().wstring().c_str());
            auto plugin_load = (plugin_load_type)::GetProcAddress(plugin, "plugin_load");
            plugin_load(api_registry);
        }
    }

    foundation::println("All plugins loaded");

    //
    foundation::println("*-*-* Running *-*-*");

    auto game = (foundation::IGame*)api_registry.first("game");
    game->run();

    return 0;
}