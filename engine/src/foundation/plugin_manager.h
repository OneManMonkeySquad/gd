#pragma once

#include "api.h"
#include "FileWatch.h"
#include <entt/entt.hpp>
#include <Windows.h>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

namespace fd
{
    struct api_registry_t;

    struct API plugin_manager_t
    {
        using plugin_t = HMODULE;
        using plugin_fix_runtime_t = void(*)(entt::locator<entt::meta_ctx>::node_type foo);
        using plugin_load_t = void(*)(fd::api_registry_t&, bool reload, void* old_dll);
        using plugin_unload_t = void(*)(fd::api_registry_t&, bool reload);

        struct loaded_plugin_t
        {
            plugin_t handle;
            fs::path temp_dll_path;
            fs::path temp_pdb_path;
        };

        fd::api_registry_t& api_registry;
        std::unordered_map<fs::path, loaded_plugin_t> plugin_modules;
        std::mutex dirty_files_lock;
        std::vector<fs::path> dirty_files;
        std::unique_ptr<filewatch::FileWatch<std::string>> watcher;

        plugin_manager_t();

        loaded_plugin_t load_plugin(fs::path plugin_path, bool is_reload, void* old_dll);

        void unload_plugin(loaded_plugin_t plugin, bool is_reload);

        void on_file_changed(const std::string& path, const filewatch::Event event);

        static bool is_plugin_path(fs::path path);

        /// <summary>
        /// Search and load found plugins. Also, start watching directory for hot-reload.
        /// </summary>
        void init();

        void dirty_file_manually(fs::path absolute_file_path);

        void blocking_wait_until_any_plugin_changes();

        void update();

        void unload_all_plugins();
    };
}