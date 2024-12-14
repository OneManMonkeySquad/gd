module;

#include <Windows.h>

export module foundation;

import std;
import <SDL3/SDL.h>;

export import :math;
export import :global_exception_handler;
export import :print;
export import :plugin_manager;
export import :api_registry;

namespace foundation
{
    export enum class error_code {
        unknown,
    };

    export struct error {
        error_code code = error_code::unknown;
        const char* message;
    };






    // #todo this is all unused
    export using object_id = uint64_t;
    export class ITheTruth
    {
    public:
        virtual ~ITheTruth() = default;

        virtual object_id create_object_of_type(int asset_type) = 0;

        virtual const void* read(object_id id) = 0;
        virtual float get_float(void* reader, std::string property) = 0;

        virtual void* write(object_id id) = 0;
        virtual void set_float(void* writer, std::string property, float value) = 0;
        virtual void commit(void* writer) = 0;
    };

    export class IEngine
    {
    public:
        virtual ~IEngine() = default;

        virtual std::expected<void, error> init(int width, int height, const char* title) = 0;
        virtual void deinit() = 0;

        virtual SDL_Window* window() = 0;
        virtual SDL_Renderer* renderer() = 0;
    };

    export using texture_handle = size_t;

    export class ISpriteManager
    {
    public:
        virtual ~ISpriteManager() = default;

        virtual std::expected<texture_handle, error> load_sprite(std::string path) = 0;
        virtual SDL_Texture* texture(texture_handle handle) const = 0;
    };

    export class IGame
    {
    public:
        virtual ~IGame() = default;

        virtual void start() = 0;
        virtual bool run_once() = 0;
        virtual void exit() = 0;
    };
}