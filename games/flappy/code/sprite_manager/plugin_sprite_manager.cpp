import foundation;
import <SDL3/SDL.h>;
import <SDL3_image/SDL_image.h>;
import std;

class MySpriteManager : public foundation::ISpriteManager
{
public:
    foundation::api_registry& api;
    // SDL_Renderer* renderer = nullptr;
    std::vector<SDL_Texture*> textures;
    std::vector<std::string> texturePaths;

    MySpriteManager(foundation::api_registry& api) : api(api)
    {
    }

    std::expected<foundation::texture_handle, foundation::error> load_sprite(std::string path) override
    {
        auto engine = (foundation::IEngine*)api.first("engine");

        // lookup cached
        for (size_t i = 0; i < texturePaths.size(); ++i)
        {
            if (texturePaths[i] == path)
                return i;
        }

        // load
        SDL_Surface* surface = IMG_Load((get_assets_path() + "\\" + path).c_str());
        if (!surface)
            return std::unexpected(foundation::error{ .message = SDL_GetError() });

        SDL_Texture* texture = SDL_CreateTextureFromSurface(engine->renderer(), surface);
        if (!texture)
            return std::unexpected(foundation::error{ .message = SDL_GetError() });

        SDL_DestroySurface(surface);

        auto textureIdx = textures.size();
        textures.push_back(texture);
        texturePaths.push_back(path);
        return textureIdx;
    }

    SDL_Texture* texture(foundation::texture_handle handle) const override
    {
        return textures[handle];
    }

    std::string get_assets_path() const {
        return std::format("{}\\assets", SDL_GetBasePath());
    }
};

namespace
{
    MySpriteManager* foo;
}

extern "C" __declspec(dllexport) void plugin_load(foundation::api_registry& api, bool reload)
{
    foo = new MySpriteManager(api);
    api.add("sprite_manager", foo);
}

extern "C" __declspec(dllexport) void plugin_unload(foundation::api_registry& api, bool reload)
{
    api.remove(foo);
    delete foo;
}