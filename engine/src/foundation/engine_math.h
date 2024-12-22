#pragma once

namespace fd
{
    struct float2 {
        float x;
        float y;
    };

    constexpr float2& operator*=(float2& lhs, const float rhs)
    {
        lhs.x *= rhs;
        lhs.y *= rhs;
        return lhs;
    }

    constexpr float2& operator+=(float2& lhs, const float rhs)
    {
        lhs.x += rhs;
        lhs.y += rhs;
        return lhs;
    }

    constexpr float2& operator+=(float2& lhs, const float2& rhs)
    {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        return lhs;
    }

    constexpr float2 operator*(float2 lhs, const float rhs)
    {
        return lhs *= rhs;
    }
}