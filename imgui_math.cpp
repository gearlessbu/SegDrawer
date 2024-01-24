#include "imgui_math.hpp"

namespace Bézier
{
    // 根据当前点pt,前一点p1, 后一点p2计算当前点对应的控制点control1 control2
    void getControlPoint(const glm::vec2 &pt, const glm::vec2 &p1, const glm::vec2 &p2,
                         glm::vec2 &control1, glm::vec2 &control2, float ratio)
    {
        float length1 = (p1 - pt).length();
        float length2 = (p2 - pt).length();

        float v = length2 / (length1 + 0.000001);

        glm::vec2 delta;
        if (v > 1)
        {
            delta = p1 - (pt + (p2 - pt) / (v + 0.000001f));
        }
        else
        {
            delta = pt + (p1 - pt) * v - p2;
        }

        delta *= ratio;
        control1 = pt + delta;
        control2 = pt - delta;
    }

    void parseBezier(const glm::vec2 &start, const glm::vec2 &end,
                     const glm::vec2 &control1, const glm::vec2 &control2, int count, std::vector<glm::vec2> &outPoints)
    {
        // if (count < 0)
        //     count = Bezier3DefaultPointNum;

        outPoints.push_back(start);
        for (int i = 1; i <= count; i++)
        {
            float st = (float)i / (count + 1);
            float dt = (1 - st);

            // 二次项
            float st2 = st * st;
            float dt2 = dt * dt;

            float t0 = dt * dt2;
            float t1 = dt2 * st * 3;
            float t2 = dt * st2 * 3;
            float t3 = st * st2;

            outPoints.push_back(start * t0 + control1 * t1 + control2 * t2 + end * t3);
        }
    }

    void _parseBezier(const glm::vec2 &start, const glm::vec2 &end,
                      const glm::vec2 &control1, const glm::vec2 &control2, int count, std::vector<glm::vec2> &outPoints)
    {
        // if (count < 0)
        //     count = Bezier3DefaultPointNum;

        outPoints.push_back(start);
        for (int i = 1; i <= count; i++)
        {
            float st = (float)i / (count + 1);
            float dt = (1 - st);

            // 二次项
            float st2 = st * st;
            float dt2 = dt * dt;

            float t0 = dt * dt2;
            float t1 = dt2 * st * 3;
            float t2 = dt * st2 * 3;
            float t3 = st * st2;

            outPoints.push_back(start * t0 + control1 * t1 + control2 * t2 + end * t3);
        }

        outPoints.push_back(end);
    }

    void parsePolyline(const std::vector<glm::vec2> &points, int count, std::vector<glm::vec2> &outPoints, float ratio)
    {
        // printf("jinitaimei\n");
        std::vector<glm::vec2>::size_type pointsSize = points.size();
        if (pointsSize < 3) // 插值至少需要三点
        {
            for (int i = 0; i < pointsSize; i++)
            {
                outPoints.push_back(points[i]);
            }
        }
        else
        {
            glm::vec2 control1, control2; // 顶点对应的前后控制点

            getControlPoint(points[1], points[0], points[2], control1, control2, ratio); // 首端
            parseBezier(points[0], points[1], points[0], control1, count, outPoints);

            for (int i = 2; i < pointsSize - 1; i++) // 根据中间各点生成与前一点连线
            {
                glm::vec2 lastControl = control2;
                getControlPoint(points[i], points[i - 1], points[i + 1], control1, control2, ratio);
                parseBezier(points[i - 1], points[i], lastControl, control1, count, outPoints);
            }

            parseBezier(points[pointsSize - 2], points[pointsSize - 1], control2, points[pointsSize - 1], count, outPoints);
            outPoints.push_back(points[pointsSize - 1]);
        }
    }

    void parsePolygon(const std::vector<glm::vec2> &points, int count, std::vector<glm::vec2> &outPoints, float ratio)
    {
        std::vector<glm::vec2>::size_type pointsSize = points.size();
        if (pointsSize < 3) // 插值至少需要三点
        {
            for (int i = 0; i < pointsSize; i++)
            {
                outPoints.push_back(points[i]);
            }
        }
        else
        {
            glm::vec2 control1, control2; // 顶点对应的前后控制点
            glm::vec2 firstControl;
            glm::vec2 lastControl;

            // 首尾
            getControlPoint(points[pointsSize - 1], points[pointsSize - 2], points[0], firstControl, lastControl, ratio);
            getControlPoint(points[0], points[pointsSize - 1], points[1], control1, control2, ratio);
            parseBezier(points[pointsSize - 1], points[0], lastControl, control1, count, outPoints);

            for (int i = 1; i < pointsSize - 1; i++) // 根据中间各点，生成与前一点连线
            {
                lastControl = control2;
                getControlPoint(points[i], points[i - 1], points[i + 1], control1, control2, ratio);
                parseBezier(points[i - 1], points[i], lastControl, control1, count, outPoints);
            }

            _parseBezier(points[pointsSize - 2], points[pointsSize - 1], control2, firstControl, count, outPoints); // 末端
            outPoints.pop_back();
        }
    }
}