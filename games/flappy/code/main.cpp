#include "FileWatch.hpp"
import <SDL3\SDL_filesystem.h>;
import <windows.h>;
import foundation;
import std;

namespace fs = std::filesystem;



struct plugin_manager
{
    struct loaded_plugin
    {
        HMODULE handle;
        fs::path temp_dll_path;
    };

    using plugin_load_t = void(*)(foundation::api_registry&);
    using plugin_unload_t = void(*)();

    foundation::api_registry& api_registry;
    std::unordered_map<fs::path, loaded_plugin> plugin_modules;
    std::mutex dirty_files_lock;
    std::vector<fs::path> dirty_files;
    std::unique_ptr<filewatch::FileWatch<std::string>> watcher;

    plugin_manager(foundation::api_registry& api_registry) : api_registry(api_registry) {}

    bool is_plugin_path(fs::path path)
    {
        const auto filename = path.filename().string();
        bool is_plugin = filename.starts_with("plugin_") && filename.ends_with(".dll");
        return is_plugin;
    }

    void on_file_changed(const std::string& path, const filewatch::Event event)
    {
        if (event != filewatch::Event::modified)
            return;

        std::scoped_lock lock{ dirty_files_lock };
        dirty_files.push_back(fs::path{ SDL_GetBasePath() } / path);
        // foundation::println("{} {}", (fs::path{ SDL_GetBasePath() } / path).string(), filewatch::event_to_string(event));
    }

    void init()
    {
        watcher = std::make_unique<filewatch::FileWatch<std::string>>(SDL_GetBasePath(),
            [&](const std::string& path, const filewatch::Event event) { on_file_changed(path, event); });

        // Load plugins
        for (const auto& dir_entry : fs::directory_iterator{ SDL_GetBasePath() })
        {
            if (is_plugin_path(dir_entry.path()))
            {
                foundation::println("Loading plugin '{}'...", dir_entry.path().string());

                auto temp_plugin_path = dir_entry.path();
                temp_plugin_path.replace_filename(std::format("_loaded_{}_{}", std::rand(), temp_plugin_path.filename().string()));
                foundation::println("   Temp path '{}'", temp_plugin_path.string());

                if (!::CopyFileW(dir_entry.path().wstring().c_str(), temp_plugin_path.wstring().c_str(), false))
                    throw std::system_error(std::error_code(::GetLastError(), std::system_category()));

                auto plugin = ::LoadLibraryW(temp_plugin_path.wstring().c_str());

                auto plugin_load = (plugin_load_t)::GetProcAddress(plugin, "plugin_load");
                plugin_load(api_registry);

                loaded_plugin entry;
                entry.handle = plugin;
                entry.temp_dll_path = temp_plugin_path;

                plugin_modules.emplace(dir_entry.path(), entry);
            }
        }

        foundation::println("Plugins successfully loaded");
    }

    void update()
    {
        // reload dirty files
        std::vector<fs::path> dirty_files_copy;
        if (dirty_files_lock.try_lock()) {
            dirty_files_copy = dirty_files;
            dirty_files.clear();
            dirty_files_lock.unlock();
        }

        if (!dirty_files_copy.empty())
        {
            for (const auto& dirty_file_path : dirty_files_copy)
            {
                if (plugin_modules.contains(dirty_file_path))
                {
                    foundation::println("Plugin changed '{}'", dirty_file_path.string());

                    // load
                    foundation::println("   Load new plugin");

                    auto temp_plugin_path = dirty_file_path;
                    temp_plugin_path.replace_filename(std::format("_loaded_{}_{}", std::rand(), temp_plugin_path.filename().string()));
                    foundation::println("   Temp path '{}'", temp_plugin_path.string());

                    if (!::CopyFileW(dirty_file_path.wstring().c_str(), temp_plugin_path.wstring().c_str(), false))
                        continue; // fails if the dll is not done building yet

                    auto plugin = ::LoadLibraryW(temp_plugin_path.wstring().c_str());
                    if (plugin == nullptr)
                        throw std::system_error(std::error_code(::GetLastError(), std::system_category()));

                    auto plugin_load = (plugin_load_t)::GetProcAddress(plugin, "plugin_load");
                    plugin_load(api_registry);

                    // free
                    foundation::println("   Unload old plugin");

                    auto loaded_entry = plugin_modules[dirty_file_path];

                    auto plugin_unload = (plugin_unload_t)::GetProcAddress(loaded_entry.handle, "plugin_unload");
                    plugin_unload(); // #todo pass pointer to new instance to transfer data

                    ::FreeLibrary(loaded_entry.handle);

                    // update entry
                    loaded_plugin entry2;
                    entry2.handle = plugin;
                    entry2.temp_dll_path = temp_plugin_path;

                    plugin_modules[dirty_file_path] = entry2;

                    foundation::println("Plugin reload successful");
                }
            }

            dirty_files_copy.clear();
        }
    }

    void exit()
    {
        // unload all modules
        for (auto [path, entry] : plugin_modules)
        {
            foundation::println("Unloading plugin '{}'...", path.string());

            auto plugin_unload = (plugin_unload_t)::GetProcAddress(entry.handle, "plugin_unload");
            plugin_unload();

            ::FreeLibrary(entry.handle);

            if (!::DeleteFileW(entry.temp_dll_path.wstring().c_str()))
                throw std::exception();
        }

        foundation::println("Plugins successfully unloaded");
    }
};

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    DWORD exceptionCode = exceptionInfo->ExceptionRecord->ExceptionCode;
    const void* exceptionAddress = exceptionInfo->ExceptionRecord->ExceptionAddress;

    auto ExceptionCodeToString = [](DWORD code) {
        switch (code) {
        case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
        default: return "Unknown Exception";
        }
        };

    auto st = std::stacktrace::current();



    foundation::println("!!!!! CRASH !!!!!");
    foundation::println("code={} {}", exceptionCode, ExceptionCodeToString(exceptionCode));

    auto it = std::begin(st);
    std::advance(it, 7); // skip handler
    for (; it != std::end(st); ++it)
    {
        foundation::println("{}({}): {}", it->source_file(), it->source_line(), it->description());
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
    SetUnhandledExceptionFilter(ExceptionHandler);

    foundation::clear_log();
    foundation::println("*-*-* INIT *-*-*");

#ifdef _WIN32
    std::setlocale(2, ".UTF8");
#endif

    foundation::api_registry api_registry;

    plugin_manager plugin_manager{ api_registry };
    plugin_manager.init();

    //
    foundation::println("*-*-* RUN *-*-*");

    {
        auto game = (foundation::IGame*)api_registry.first("game");
        game->start();
    }

    while (true)
    {
        auto game = (foundation::IGame*)nullptr;// api_registry.first("game");
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