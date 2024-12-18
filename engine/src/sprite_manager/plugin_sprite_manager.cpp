import foundation;
import <SDL3/SDL.h>;
import <SDL3_image/SDL_image.h>;
import std;

namespace
{
    foundation::engine_t* engine;
    foundation::job_system_t* job_system;
    std::vector<SDL_Texture*> textures;
    std::vector<std::string> texture_paths;

    std::string get_assets_path() {
        return std::format("{}\\assets", SDL_GetBasePath());
    }

    struct load_texture_data_t
    {
        std::string path;
        SDL_Texture* texture;
    };

    void load_texture_job(void* data)
    {
        auto job_data = (load_texture_data_t*)data;

        SDL_Surface* surface = IMG_Load(job_data->path.c_str());
        if (!surface)
            ;// return std::unexpected(foundation::error_t{ .message = SDL_GetError() });

        SDL_Texture* texture = SDL_CreateTextureFromSurface(engine->renderer(), surface);
        if (!texture)
            ;// return std::unexpected(foundation::error_t{ .message = SDL_GetError() });

        SDL_DestroySurface(surface);

        job_data->texture = texture;
    }

    std::expected<foundation::texture_handle_t, foundation::error_t> load_sprite(std::string path)
    {
        // lookup cached
        for (size_t i = 0; i < texture_paths.size(); ++i)
        {
            if (texture_paths[i] == path)
                return i;
        }

        // load
        load_texture_data_t data;
        data.path = get_assets_path() + "\\" + path;
        auto job = job_system->run_jobs({ foundation::jobdecl_t{ &load_texture_job, &data } });
        job_system->wait_for_counter(job, 0);

        auto textureIdx = textures.size();
        textures.push_back(data.texture);
        texture_paths.push_back(path);
        return textureIdx;
    }

    SDL_Texture* texture(foundation::texture_handle_t handle)
    {
        return textures[handle];
    }
}

extern "C" __declspec(dllexport) void plugin_load(foundation::api_registry& api, bool reload)
{
    engine = api.get<foundation::engine_t>();
    job_system = api.get<foundation::job_system_t>();

    foundation::sprite_manager_t sm;
    sm.load_sprite = &load_sprite;
    sm.texture = &texture;

    api.set(sm);
}

extern "C" __declspec(dllexport) void plugin_unload(foundation::api_registry& api, bool reload)
{
    // api.remove(foo);
    // delete foo;
}