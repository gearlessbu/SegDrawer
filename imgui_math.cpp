#include "imgui_math.hpp"

float cross(const ImVec2 &a, const ImVec2 &b)
{
    return a.x * b.y - a.y * b.x;
}

ImVec2 subtraction(const ImVec2 &a, const ImVec2 &b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

bool intersection(const ImVec2 &a, const ImVec2 &b, const ImVec2 &c, const ImVec2 &d)
{
    // 快速排斥实验
    if (std::max(c.x, d.x) < std::min(a.x, b.x) || std::max(a.x, b.x) < std::min(c.x, d.x) || std::max(c.y, d.y) < std::min(a.y, b.y) || std::max(a.y, b.y) < std::min(c.y, d.y))
    {
        return false;
    }
    // 跨立实验
    if (cross(subtraction(a, d), subtraction(c, d)) * cross(subtraction(b, d), subtraction(c, d)) > 0 || cross(subtraction(d, b), subtraction(a, b)) * cross(subtraction(c, b), subtraction(a, b)) > 0)
    {
        return false;
    }
    return true;
}

inline float squaredNorm(const ImVec2 &a)
{
    return a.x * a.x + a.y * a.y;
}

inline float squaredDistance(const ImVec2 &a, const ImVec2 &b)
{
    ImVec2 p(a.x - b.x, a.y - b.y);
    return p.x * p.x + p.y * p.y;
}

inline float distance(const ImVec2 &a, const ImVec2 &b)
{
    ImVec2 p(a.x - b.x, a.y - b.y);
    return sqrtf(p.x * p.x + p.y * p.y);
}
