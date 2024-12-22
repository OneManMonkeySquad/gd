#include "foundation/foundation.h"
#include "foundation/print.h"
#include "foundation/api_registry.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

//import <SDL3/SDL.h>;
//import <SDL3_image/SDL_image.h>;
//import std;

namespace
{
    struct load_texture_data_t
    {
        fd::window_t window;
        std::string path;

        SDL_Texture* texture;
    };

    fd::platform_t* platform;
    fd::job_system_t* job_system;
    std::vector<SDL_Texture*> textures;
    std::vector<std::string> texture_paths;

    std::string get_assets_path() {
        return std::format("{}\\assets", SDL_GetBasePath());
    }

    void load_texture_job(void* data)
    {
        auto job_data = (load_texture_data_t*)data;

        fd::println("load texture '{}'", job_data->path.c_str());

        SDL_Surface* surface = IMG_Load(job_data->path.c_str()); // todo DONT BLOCK JOB ASSHOLE
        if (!surface)
            throw fd::error_t(SDL_GetError());

        SDL_Texture* texture = SDL_CreateTextureFromSurface(platform->get_sdl_renderer(job_data->window), surface);
        if (!texture)
            throw fd::error_t(SDL_GetError());

        SDL_DestroySurface(surface);

        job_data->texture = texture;
    }

    std::expected<fd::texture_t, fd::error_t> load_sprite(std::string path, fd::window_t window)
    {
        // lookup cached
        for (size_t i = 0; i < texture_paths.size(); ++i)
        {
            if (texture_paths[i] == path)
                return (fd::texture_t)i;
        }

        // load
        load_texture_data_t data;
        data.window = window;
        data.path = get_assets_path() + "\\" + path;
        auto job = job_system->run_jobs({ fd::jobdecl_t{ &load_texture_job, &data } });
        job_system->wait_for_counter(job, 0);

        auto texture_idx = textures.size();
        textures.push_back(data.texture);
        texture_paths.push_back(path);
        return (fd::texture_t)texture_idx;
    }

    SDL_Texture* texture(fd::texture_t handle)
    {
        return textures[(size_t)handle];
    }
}

extern "C" __declspec(dllexport) void load_plugin(fd::api_registry_t& api, bool reload, void* old_dll)
{
    platform = api.get<fd::platform_t>();
    job_system = api.get<fd::job_system_t>();

    fd::sprite_manager_t sm;
    sm.load_sprite = &load_sprite;
    sm.texture = &texture;

    api.set(sm);
}

extern "C" __declspec(dllexport) void unload_plugin(fd::api_registry_t& api, bool reload)
{
    // api.remove(foo);
    // delete foo;
}