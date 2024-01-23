#ifndef CANVAS_HPP
#define CANVAS_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <functional>

#include "small_imgui/imgui.h"
#include "imgui_math.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

void Canvasrender();

template <typename T>
T nearest_number_cell(const T a_value, const T cell)
{
    T cell_remain = fmod(a_value, cell);
    if (cell_remain > 0.5)
        return cell * ceil(a_value / cell);
    else
        return cell * floor(a_value / cell);
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    int nn = n;
    std::ostringstream out;
    out << std::fixed << std::setprecision(nn) << a_value;
    return out.str();
}

struct edge_type
{
    ImVec2 v, w;
    bool isBad = false;
};

struct triangle_type
{
    // std::shared_ptr<ImVec2> a, b, c;
    ImVec2 a, b, c;
    bool isBad = false;

    bool containsVertex(const ImVec2 &v) const
    {
        // return p1 == v || p2 == v || p3 == v;
        auto almost_equal = [](float x, float y, int ulp = 2)
        {
            return fabs(x - y) <= std::numeric_limits<float>::epsilon() * fabs(x + y) * static_cast<float>(ulp) || fabs(x - y) < std::numeric_limits<float>::min();
        };
        auto almost_equal_v = [&](const ImVec2 &v1, const ImVec2 &v2)
        {
            return almost_equal(v1.x, v2.x) && almost_equal(v1.y, v2.y);
        };
        return almost_equal_v(a, v) || almost_equal_v(b, v) || almost_equal_v(c, v);
    }

    bool circumCircleContains(const ImVec2 &v)
    {
        const float ab = squaredNorm(a);
        const float cd = squaredNorm(b);
        const float ef = squaredNorm(c);

        const float ax = a.x;
        const float ay = a.y;
        const float bx = b.x;
        const float by = b.y;
        const float cx = c.x;
        const float cy = c.y;

        const float circum_x = (ab * (cy - by) + cd * (ay - cy) + ef * (by - ay)) / (ax * (cy - by) + bx * (ay - cy) + cx * (by - ay));
        const float circum_y = (ab * (cx - bx) + cd * (ax - cx) + ef * (bx - ax)) / (ay * (cx - bx) + by * (ax - cx) + cy * (bx - ax));

        const ImVec2 circum(circum_x / 2, circum_y / 2);
        const float circum_radius = squaredDistance(a, circum);
        const float dist = squaredDistance(v, circum);
        return dist <= circum_radius;
    }
};

struct Canvas
{
    Canvas(const glm::ivec2 &widget_size);
    void render();
    void save_points(const std::string &file_path);
    void area_compute();
    std::vector<int> new_edge_intersect(const ImVec2 &new_point);
    void triangulate();
    void Delaunay_triangulate();
    void reset_COM_to_origin();

    glm::ivec2 m_widget_size;
    ImVector<ImVec2> points;
    std::vector<triangle_type> triangles;
    std::vector<glm::ivec3> triangles_id;

    bool finish_peri;
};

#endif