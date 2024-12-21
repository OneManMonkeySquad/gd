module;

#include <Windows.h>

export module foundation:plugin_manager;

import std;
import <SDL3/SDL.h>;
import <entt/entt.hpp>;
import "FileWatch.hpp";

import :print;
import :api_registry_t;

namespace fs = std::filesystem;

namespace fd
{
    bool cr_pdb_replace(const std::string& filename,
        const std::string& pdbname,
        std::string& orig_pdb);



    export struct plugin_manager_t
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

        plugin_manager_t(fd::api_registry_t& api_registry) : api_registry(api_registry) {}

        loaded_plugin_t load_plugin(fs::path plugin_path, bool is_reload, void* old_dll)
        {
            fd::println("Loading plugin '{}'...", plugin_path.filename().string());

            auto temp_plugin_path = plugin_path;
            temp_plugin_path.replace_filename(std::format("_{}_{}", std::rand(), temp_plugin_path.filename().string()));
            fd::println("   Temp path '{}'", temp_plugin_path.filename().string());

            if (!::CopyFileW(plugin_path.wstring().c_str(), temp_plugin_path.wstring().c_str(), false))
                throw std::system_error(std::error_code(::GetLastError(), std::system_category()));

            auto pdbPath = plugin_path;
            pdbPath.replace_extension(".pdb");

            auto temp_pdb_path = temp_plugin_path;
            temp_pdb_path.replace_extension(".pdb");
            fd::println("   Temp pdb path '{}'", temp_pdb_path.filename().string());

            // note: pdb might be missing, ignore error
            if (::CopyFileW(pdbPath.wstring().c_str(), temp_pdb_path.wstring().c_str(), false))
            {
                std::string origPdb;
                cr_pdb_replace(temp_plugin_path.string(), temp_plugin_path.filename().replace_extension(".pdb").string(), origPdb);
            }

            auto plugin = ::LoadLibraryW(temp_plugin_path.wstring().c_str());
            if (plugin == nullptr)
                throw std::system_error(std::error_code(::GetLastError(), std::system_category()));

            auto handle = entt::locator<entt::meta_ctx>::handle();

            auto plugin_fix_runtime = (plugin_fix_runtime_t)::GetProcAddress(plugin, "set_plugin_runtime");
            if (plugin_fix_runtime != nullptr)
            {
                plugin_fix_runtime(handle);
            }

            auto plugin_load = (plugin_load_t)::GetProcAddress(plugin, "load_plugin");
            plugin_load(api_registry, is_reload, old_dll);

            loaded_plugin_t entry;
            entry.handle = plugin;
            entry.temp_dll_path = temp_plugin_path;
            entry.temp_pdb_path = temp_pdb_path;
            return entry;
        }

        void unload_plugin(loaded_plugin_t plugin, bool is_reload)
        {
            auto plugin_unload = (plugin_unload_t)::GetProcAddress(plugin.handle, "unload_plugin");
            plugin_unload(api_registry, is_reload);

            ::FreeLibrary(plugin.handle);
        }

        void on_file_changed(const std::string& path, const filewatch::Event event)
        {
            if (event != filewatch::Event::modified)
                return;

            dirty_file_manually(fs::path{ SDL_GetBasePath() } / path);
            // fd::println("{} {}", (fs::path{ SDL_GetBasePath() } / path).string(), filewatch::event_to_string(event));
        }

        static bool is_plugin_path(fs::path path)
        {
            const auto filename = path.filename().string();
            bool is_plugin = filename.starts_with("plugin_") && filename.ends_with(".dll");
            return is_plugin;
        }

        /// <summary>
        /// Search and load found plugins. Also, start watching directory for hot-reload.
        /// </summary>
        void init()
        {
            watcher = std::make_unique<filewatch::FileWatch<std::string>>(SDL_GetBasePath(),
                [&](const std::string& path, const filewatch::Event event) { on_file_changed(path, event); });

            // Load plugins
            for (const auto& dir_entry : fs::directory_iterator{ SDL_GetBasePath() })
            {
                if (!is_plugin_path(dir_entry.path()))
                    continue;

                loaded_plugin_t entry = load_plugin(dir_entry.path(), false, nullptr);
                plugin_modules.emplace(dir_entry.path(), entry);
            }

            fd::println("Plugins successfully loaded");
        }

        void dirty_file_manually(fs::path absolute_file_path)
        {
            std::scoped_lock lock{ dirty_files_lock };
            dirty_files.push_back(absolute_file_path);
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
                    if (!plugin_modules.contains(dirty_file_path))
                        continue;

                    fd::println("Plugin changed '{}'", dirty_file_path.filename().string());

                    try
                    {
                        auto loaded_entry = plugin_modules[dirty_file_path];

                        auto entry2 = load_plugin(dirty_file_path, true, loaded_entry.handle);

                        fd::println("   Unload old plugin");

                        unload_plugin(loaded_entry, true);

                        // update entry
                        plugin_modules[dirty_file_path] = entry2;

                        fd::println("Plugin reload successful");
                    }
                    catch (const std::exception& e)
                    {
                        fd::println("   Plugin reload failed: {}", e.what());
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
                fd::println("Unloading plugin '{}'...", path.filename().string());

                unload_plugin(entry, false);

                // note: might fail if we are debugging
                // #todo delete on next start
                (void)::DeleteFileW(entry.temp_dll_path.wstring().c_str());

                // note: pdb might be missing, ignore error
                (void)::DeleteFileW(entry.temp_pdb_path.wstring().c_str());
            }

            fd::println("Plugins successfully unloaded");
        }
    };

    template <class T>
    static T struct_cast(void* ptr, LONG offset = 0) {
        return reinterpret_cast<T>(reinterpret_cast<intptr_t>(ptr) + offset);
    }

    // RSDS Debug Information for PDB files
    using DebugInfoSignature = DWORD;
#define CR_RSDS_SIGNATURE 'SDSR'
    struct cr_rsds_hdr {
        DebugInfoSignature signature;
        GUID guid;
        long version;
        char filename[1];
    };

    static bool cr_pe_fileoffset_rva(PIMAGE_NT_HEADERS ntHeaders, DWORD rva,
        DWORD& fileOffset) {
        bool found = false;
        auto* sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
        for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections;
            i++, sectionHeader++) {
            auto sectionSize = sectionHeader->Misc.VirtualSize;
            if ((rva >= sectionHeader->VirtualAddress) &&
                (rva < sectionHeader->VirtualAddress + sectionSize)) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }

        const int diff = static_cast<int>(sectionHeader->VirtualAddress -
            sectionHeader->PointerToRawData);
        fileOffset = rva - diff;
        return true;
    }


    static bool cr_pe_debugdir_rva(PIMAGE_OPTIONAL_HEADER optionalHeader,
        DWORD& debugDirRva, DWORD& debugDirSize) {
        if (optionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            auto optionalHeader64 =
                struct_cast<PIMAGE_OPTIONAL_HEADER64>(optionalHeader);
            debugDirRva =
                optionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]
                .VirtualAddress;
            debugDirSize =
                optionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
        }
        else {
            auto optionalHeader32 =
                struct_cast<PIMAGE_OPTIONAL_HEADER32>(optionalHeader);
            debugDirRva =
                optionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG]
                .VirtualAddress;
            debugDirSize =
                optionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
        }

        if (debugDirRva == 0 && debugDirSize == 0) {
            return true;
        }
        else if (debugDirRva == 0 || debugDirSize == 0) {
            return false;
        }

        return true;
    }

    static char* cr_pdb_find(LPBYTE imageBase, PIMAGE_DEBUG_DIRECTORY debugDir) {
        // CR_ASSERT(debugDir && imageBase);
        LPBYTE debugInfo = imageBase + debugDir->PointerToRawData;
        const auto debugInfoSize = debugDir->SizeOfData;
        if (debugInfo == 0 || debugInfoSize == 0) {
            return nullptr;
        }

        if (IsBadReadPtr(debugInfo, debugInfoSize)) {
            return nullptr;
        }

        if (debugInfoSize < sizeof(DebugInfoSignature)) {
            return nullptr;
        }

        if (debugDir->Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
            auto signature = *(DWORD*)debugInfo;
            if (signature == CR_RSDS_SIGNATURE) {
                auto* info = (cr_rsds_hdr*)(debugInfo);
                if (IsBadReadPtr(debugInfo, sizeof(cr_rsds_hdr))) {
                    return nullptr;
                }

                if (IsBadStringPtrA((const char*)info->filename, UINT_MAX)) {
                    return nullptr;
                }

                return info->filename;
            }
        }

        return nullptr;
    }

    // https://github.com/fungos/cr/blob/master/cr.h
    bool cr_pdb_replace(const std::string& filename, const std::string& pdbname,
        std::string& orig_pdb) {
        const std::string& _filename = filename;

        HANDLE fp = nullptr;
        HANDLE filemap = nullptr;
        LPVOID mem = 0;
        bool result = false;
        do {
            fp = CreateFile(_filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, nullptr);
            if ((fp == INVALID_HANDLE_VALUE) || (fp == nullptr)) {
                break;
            }

            filemap = CreateFileMapping(fp, nullptr, PAGE_READWRITE, 0, 0, nullptr);
            if (filemap == nullptr) {
                break;
            }

            mem = MapViewOfFile(filemap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (mem == nullptr) {
                break;
            }

            auto dosHeader = struct_cast<PIMAGE_DOS_HEADER>(mem);
            if (dosHeader == 0) {
                break;
            }

            if (IsBadReadPtr(dosHeader, sizeof(IMAGE_DOS_HEADER))) {
                break;
            }

            if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                break;
            }

            auto ntHeaders =
                struct_cast<PIMAGE_NT_HEADERS>(dosHeader, dosHeader->e_lfanew);
            if (ntHeaders == 0) {
                break;
            }

            if (IsBadReadPtr(ntHeaders, sizeof(ntHeaders->Signature))) {
                break;
            }

            if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
                break;
            }

            if (IsBadReadPtr(&ntHeaders->FileHeader, sizeof(IMAGE_FILE_HEADER))) {
                break;
            }

            if (IsBadReadPtr(&ntHeaders->OptionalHeader,
                ntHeaders->FileHeader.SizeOfOptionalHeader)) {
                break;
            }

            if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
                ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                break;
            }

            auto sectionHeaders = IMAGE_FIRST_SECTION(ntHeaders);
            if (IsBadReadPtr(sectionHeaders,
                ntHeaders->FileHeader.NumberOfSections *
                sizeof(IMAGE_SECTION_HEADER))) {
                break;
            }

            DWORD debugDirRva = 0;
            DWORD debugDirSize = 0;
            if (!cr_pe_debugdir_rva(&ntHeaders->OptionalHeader, debugDirRva,
                debugDirSize)) {
                break;
            }

            if (debugDirRva == 0 || debugDirSize == 0) {
                break;
            }

            DWORD debugDirOffset = 0;
            if (!cr_pe_fileoffset_rva(ntHeaders, debugDirRva, debugDirOffset)) {
                break;
            }

            auto debugDir =
                struct_cast<PIMAGE_DEBUG_DIRECTORY>(mem, debugDirOffset);
            if (debugDir == 0) {
                break;
            }

            if (IsBadReadPtr(debugDir, debugDirSize)) {
                break;
            }

            if (debugDirSize < sizeof(IMAGE_DEBUG_DIRECTORY)) {
                break;
            }

            int numEntries = debugDirSize / sizeof(IMAGE_DEBUG_DIRECTORY);
            if (numEntries == 0) {
                break;
            }

            for (int i = 1; i <= numEntries; i++, debugDir++) {
                char* pdb = cr_pdb_find((LPBYTE)mem, debugDir);
                if (pdb) {
                    auto len = strlen(pdb);
                    if (len >= strlen(pdbname.c_str())) {
                        orig_pdb = pdb;
                        memcpy_s(pdb, len, pdbname.c_str(), pdbname.length());
                        pdb[pdbname.length()] = 0;
                        result = true;
                    }
                }
            }
        } while (0);

        if (mem != nullptr) {
            UnmapViewOfFile(mem);
        }

        if (filemap != nullptr) {
            CloseHandle(filemap);
        }

        if ((fp != nullptr) && (fp != INVALID_HANDLE_VALUE)) {
            CloseHandle(fp);
        }

        return result;
    }
}