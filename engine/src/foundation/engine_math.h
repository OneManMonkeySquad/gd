#pragma once

namespace fd
{
    struct float2 {
        const static float2 zero;

        float x;
        float y;
    };

    const float2 float2::zero = float2{ 0, 0 };

    constexpr float2& operator*=(float2& lhs, const float rhs)
    {
        lhs.x *= rhs;
        lhs.y *= rhs;
        return lhs;
    }

    /*constexpr float2& operator+=(float2& lhs, const float rhs)
    {
        lhs.x += rhs;
        lhs.y += rhs;
        return lhs;
    }*/

    constexpr float2& operator+=(float2& lhs, const float2& rhs)
    {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        return lhs;
    }

    constexpr float2& operator-=(float2& lhs, const float2& rhs)
    {
        lhs.x -= rhs.x;
        lhs.y -= rhs.y;
        return lhs;
    }

    constexpr float2 operator*(const float2 lhs, const float rhs)
    {
        float2 temp = lhs;
        temp *= rhs;
        return temp;
    }

    constexpr float2 operator+(const float2 lhs, const float2 rhs)
    {
        float2 temp = lhs;
        temp += rhs;
        return temp;
    }

    constexpr float2 operator-(const float2 lhs, const float2 rhs)
    {
        float2 temp = lhs;
        temp -= rhs;
        return temp;
    }

    constexpr float2 lerp(float2 from, float2 to, float a)
    {
        return from + (to - from) * a;
    }
}