#include "canvas.hpp"

void test_dln()
{
    auto cc = Canvas(glm::ivec2{4, 6});
    cc.points.push_back(ImVec2{2, 2});
    cc.points.push_back(ImVec2{-2, 2});
    cc.points.push_back(ImVec2{-2, -2});
    cc.points.push_back(ImVec2{2, -2});
    cc.Delaunay_triangulate();
}

int main()
{
    test_dln();
    return 0;
}