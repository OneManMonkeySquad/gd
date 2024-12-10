export module foundation;

import std;
import <SDL3/SDL.h>;
import <Windows.h>;

namespace foundation
{
    export template<typename... Args>
        void println(std::format_string<Args...> fmt, Args&&... args)
    {
        auto str = std::format(fmt, std::forward<Args>(args)...);
        str += '\n';
        ::OutputDebugStringA(str.c_str());
    }



    export enum class error_code {
        unknown,
    };

    export struct error {
        error_code code = error_code::unknown;
        const char* message;
    };



    export struct api_registry
    {
        std::vector<std::pair<std::string, void*>> foo;

        void add(std::string name, void* api)
        {
            foo.push_back(std::make_pair(name, api));
        }

        void* first(std::string name)
        {
            for (auto& pair : foo)
            {
                if (pair.first == name)
                    return pair.second;
            }
            return nullptr;
        }
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

        virtual std::expected<void, error> init() = 0;
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

        virtual void run() = 0;
    };





    export struct float2 {
        float x;
        float y;
    };

    export constexpr float2& operator*=(float2& lhs, const float rhs)
    {
        lhs.x *= rhs;
        lhs.y *= rhs;
        return lhs;
    }

    export constexpr float2& operator+=(float2& lhs, const float rhs)
    {
        lhs.x += rhs;
        lhs.y += rhs;
        return lhs;
    }

    export constexpr float2& operator+=(float2& lhs, const float2& rhs)
    {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        return lhs;
    }

    export constexpr float2 operator*(float2 lhs, const float rhs)
    {
        return lhs *= rhs;
    }
}