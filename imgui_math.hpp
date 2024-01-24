#ifndef IMGUI_MATH
#define IMGUI_MATH

#include "small_imgui/imgui.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <math.h>
#include <vector>
#include <cstdio>

inline float cross(const ImVec2 &a, const ImVec2 &b)
{
    return a.x * b.y - a.y * b.x;
}

inline ImVec2 subtraction(const ImVec2 &a, const ImVec2 &b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

inline std::vector<float> Culculate_Binomial_Coefficient(int n)
{
    int k, j;
    std::vector<float> C(n + 1);

    for (k = 0; k <= n; k++)
    {
        C[k] = 1;
        for (j = n; j >= k + 1; j--) // 1*n*(n-1)*(n-2)*...*(k+2)*(k+1)
        {
            C[k] *= j;
        }
        for (j = n - k; j >= 2; j--) // 上式 / ( (n-k)*(n-k-1)*...*3*2*1 )
        {
            C[k] /= j;
        }
    }
    return C;
}

inline bool in_edges_range(const ImVec2 &center, const ImVec2 &next_clock_point, const ImVec2 &prev_clock_point, const ImVec2 query_point)
{
    float next_point_angle = std::atan2(next_clock_point.y - center.y, next_clock_point.x - center.x);
    float prev_point_angle = std::atan2(prev_clock_point.y - center.y, prev_clock_point.x - center.x);
    float query_point_angle = std::atan2(query_point.y - center.y, query_point.x - center.x);
    // if ((next_point_angle >= 0 && prev_point_angle >= 0)||(next_point_angle <= 0 && prev_point_angle <= 0))
    if (next_point_angle > prev_point_angle)
    {
        return next_point_angle - 1e-4 < query_point_angle && query_point_angle < prev_point_angle + 1e-4;
    }
    else
    {
        // assert(next_point_angle > 0)
        return prev_point_angle - 1e-4 < query_point_angle || query_point_angle < next_point_angle + 1e-4;
    }
}

inline float triangle_area(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3)
{
    float x1 = p3.x - p1.x;
    float y1 = p3.y - p1.y;
    float x2 = p2.x - p1.x;
    float y2 = p2.y - p1.y;
    return fabsf(x1 * y2 - x2 * y1);
}

inline bool in_triangle(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &query)
{
    // glm::mat2 A = glm::mat2(p1.x - p3.x, p2.x - p3.x,
    //                         p1.y - p3.y, p2.y - p3.y);
    // // A
    // // A.col(0) = points.col(1) - points.col(0);
    // // A.col(1) = points.col(2) - points.col(0);
    // glm::vec2 b = glm::vec2{query.x - p3.y, query.y - p3.y};
    // glm::vec2 x = glm::inverse(A) * b;
    // // const Vector2r b = point - points.col(0);
    // // const Vector2r x = A.inverse() * b;inverse
    // printf("x:(%f, %f)\n", x.x, x.y);
    // return std::min(x.x, x.y) >= 0.0f && std::max(x.x, x.y) <= 1.0f && x.x + x.y <= 1.0f;
    // // return x.minCoeff() >= 0 && x.maxCoeff() <= 1 && x.sum() <= 1;

    float x1 = query.x - p1.x;
    float y1 = query.y - p1.y;
    float x2 = p3.x - p1.x;
    float y2 = p3.y - p1.y;
    float x3 = p2.x - p1.x;
    float y3 = p2.y - p1.y;
    float u = (x1 * y3 - x3 * y1) / (x2 * y3 - x3 * y2);
    float v = (x1 * y2 - x2 * y1) / (x3 * y2 - x2 * y3);
    return u > 0 && v > 0 && u + v < 1;
}

inline bool in_edges_range(const ImVec2 &center, const ImVec2 &next_clock_point, const ImVec2 &prev_clock_point,
                           const ImVec2 query_point1, const ImVec2 query_point2)
{
    float next_point_angle = std::atan2(next_clock_point.y - center.y, next_clock_point.x - center.x);
    float prev_point_angle = std::atan2(prev_clock_point.y - center.y, prev_clock_point.x - center.x);
    float query_point_angle1 = std::atan2(query_point1.y - center.y, query_point1.x - center.x);
    float query_point_angle2 = std::atan2(query_point2.y - center.y, query_point2.x - center.x);
    // printf("arctan2(1,1)=%f\n", std::atan2(1.0f, 1.0f));
    // printf("na=%f, pa=%f, qa1=%f, qa2=%f\n", next_point_angle, prev_point_angle, query_point_angle1, query_point_angle2);
    // if ((next_point_angle >= 0 && prev_point_angle >= 0)||(next_point_angle <= 0 && prev_point_angle <= 0))
    auto close = [](float a, float b)
    {
        float dif = fabsf(a - b);
        return dif < 1e-4 && fabsf(dif - 2 * M_PI) < 1e-4;
    };
    auto to_2pi = [](float a)
    {
        while (a > 2 * M_PI)
        {
            a -= 2 * M_PI;
        }
        while (a < 0)
        {
            a += 2 * M_PI;
        }
        return a;
    };
    if ((close(next_point_angle, query_point_angle1) && close(prev_point_angle, query_point_angle2)) || (close(next_point_angle, query_point_angle2) && close(prev_point_angle, query_point_angle1)))
    {
        if (to_2pi(next_point_angle - prev_point_angle) < M_PI)
            return true;
        else
            return false;
    }
    if (next_point_angle > prev_point_angle)
    {
        bool in1 = prev_point_angle - 1e-4 < query_point_angle1 && query_point_angle1 < next_point_angle + 1e-4;
        bool in2 = prev_point_angle - 1e-4 < query_point_angle2 && query_point_angle2 < next_point_angle + 1e-4;
        // printf("jinitaimei\n");
        // if (in1 && in2)
        //     printf("jinitaimei\n");
        return in1 && in2;
    }
    else
    {
        // assert(next_point_angle > 0)
        bool in1 = prev_point_angle - 1e-4 < query_point_angle1 || query_point_angle1 < next_point_angle + 1e-4;
        bool in2 = prev_point_angle - 1e-4 < query_point_angle2 || query_point_angle2 < next_point_angle + 1e-4;
        return in1 && in2;
    }
}

inline bool intersection(const ImVec2 &a, const ImVec2 &b, const ImVec2 &c, const ImVec2 &d)
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

namespace Bézier
{
    // 根据当前点pt,前一点p1, 后一点p2计算当前点对应的控制点control1 control2
    void getControlPoint(const glm::vec2 &pt, const glm::vec2 &p1, const glm::vec2 &p2,
                         glm::vec2 &control1, glm::vec2 &control2, float ratio);

    void parseBezier(const glm::vec2 &start, const glm::vec2 &end,
                     const glm::vec2 &control1, const glm::vec2 &control2, int count, std::vector<glm::vec2> &outPoints);

    void _parseBezier(const glm::vec2 &start, const glm::vec2 &end,
                      const glm::vec2 &control1, const glm::vec2 &control2, int count, std::vector<glm::vec2> &outPoints);

    void parsePolyline(const std::vector<glm::vec2> &points, int count, std::vector<glm::vec2> &outPoints, float ratio);

    void parsePolygon(const std::vector<glm::vec2> &points, int count, std::vector<glm::vec2> &outPoints, float ratio);
}

#endif