export module foundation:math;

namespace foundation
{
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