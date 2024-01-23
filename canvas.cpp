#include "canvas.hpp"
#include <fstream>

void *GImGuiDemoMarkerCallbackUserData_ = NULL;
typedef void (*ImGuiDemoMarkerCallback_)(const char *file, int line, const char *section, void *user_data);
ImGuiDemoMarkerCallback_ GImGuiDemoMarkerCallback_ = NULL;
#define IMGUI_DEMO_MARKER_(section)                                                                    \
    do                                                                                                 \
    {                                                                                                  \
        if (GImGuiDemoMarkerCallback_ != NULL)                                                         \
            GImGuiDemoMarkerCallback_(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData_); \
    } while (0)

static void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void ShowFileWindow(bool *p_open)
{
    // const ImGuiViewport *viewport = ImGui::GetMainViewport();
    // ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x / 2 - 300, viewport->WorkSize.y / 2 - 300));
    // ImGui::SetNextWindowSize(ImVec2{600, 600});
    // static ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
    // if (!ImGui::Begin("Example: Custom rendering", p_open, flags))
    // {
    //     ImGui::End();
    //     return;
    // }
    // // IMGUI_DEMO_MARKER_("Examples/Custom Rendering");
    // // ImGui::Text("jinitiamei");
    // if (ImGui::BeginTabBar("##TabBar"))
    // {
    //     if (ImGui::BeginTabItem("Primitives"))
    //     {
    //         ImGui::EndTabItem();
    //     }
    //     ImGui::EndTabBar();
    // }

    // ImGui::End();
    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!");
        ImGui::Separator();

        // static int unused_i = 0;
        // ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

        static bool dont_ask_me_next_time = false;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
        ImGui::PopStyleVar();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

Canvas::Canvas(const glm::ivec2 &widget_size)
{
    points.clear();
    finish_peri = false;
}

void Canvas::save_points(const std::string &file_path)
{
    std::ofstream f(file_path);
    for (auto p : points)
    {
        f << "v " << p.x << " " << p.y << " 0";
        f.write("\n", sizeof(char));
    }
    for (int e = 0; e < points.Size - 1; ++e)
    {
        f << "f " << e + 1 << " " << e + 2 << " " << 1;
        f.write("\n", sizeof(char));
    }
    f << "f " << points.Size << " " << 1 << " " << 1;
    f.write("\n", sizeof(char));
    f.close();
}

std::vector<int> Canvas::new_edge_intersect(const ImVec2 &new_point)
{
    std::vector<int> ret;
    if (points.Size < 3)
        return ret;
    for (int idx = 0; idx < points.Size - 2; ++idx)
    {
        if (intersection(points[idx], points[idx + 1], points[points.Size - 1], new_point))
            ret.push_back(idx);
    }
}

void Canvas::triangulate()
{
    triangles_id.clear();
    triangles.clear();
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
    auto get_angle = [this, to_2pi](int center, int next_clock_id, int prev_clock_id)
    {
        float next_point_angle = std::atan2(points[next_clock_id].y - points[center].y, points[next_clock_id].x - points[center].x);
        float prev_point_angle = std::atan2(points[prev_clock_id].y - points[center].y, points[prev_clock_id].x - points[center].x);
        return to_2pi(next_point_angle - prev_point_angle);
    };
    std::vector<int> contour_ids;
    for (int i = 0; i < points.Size; ++i)
    {
        contour_ids.push_back(i);
    }
    // if (in_triangle(ImVec2(0, 0), ImVec2(3, 0), ImVec2(0, 3), ImVec2(1.5, 1.4)))
    // {
    //     printf("hahahah\n");
    // }
    for (int i = 0; i < points.Size - 2; ++i)
    {
        for (auto p_id = contour_ids.begin(); p_id < contour_ids.end(); ++p_id)
        {
            // contour_ids.erase()
            int center_id = *p_id;
            int next_clock_id = -1;
            int prev_clock_id = -1;
            if (p_id == contour_ids.begin())
            {
                next_clock_id = *(contour_ids.end() - 1);
            }
            else
                next_clock_id = *(p_id - 1);
            if (p_id == contour_ids.end() - 1)
            {
                prev_clock_id = *contour_ids.begin();
            }
            else
                prev_clock_id = *(p_id + 1);
            if (get_angle(center_id, next_clock_id, prev_clock_id) < M_PI)
            {
                bool overlap_tri = false;
                for (int tmp_id = 0; tmp_id < points.Size; ++tmp_id)
                {
                    if (tmp_id == center_id)
                        continue;
                    if (tmp_id == next_clock_id)
                        continue;
                    if (tmp_id == prev_clock_id)
                        continue;
                    if (in_triangle(points[center_id], points[next_clock_id], points[prev_clock_id], points[tmp_id]))
                    {
                        overlap_tri = true;
                        break;
                    }
                }
                if (overlap_tri)
                    continue;
                triangles_id.push_back(glm::ivec3{center_id, next_clock_id, prev_clock_id});
                contour_ids.erase(p_id);
                break;
            }
        }
    }
    for (auto tri_id : triangles_id)
    {
        // printf("(%d, %d, %d)\n", tri_id[0], tri_id[1], tri_id[2]);
        triangles.push_back(triangle_type{points[tri_id[0]], points[tri_id[1]], points[tri_id[2]]});
    }
}

void Canvas::reset_COM_to_origin()
{
    ImVector<ImVec2> coms;
    ImVector<float> masses;
    float mass_sum = 0.0f;
    for (auto tri : triangles)
    {
        ImVec2 com = ImVec2{(tri.a.x + tri.b.x + tri.c.x) / 3.0f, (tri.a.y + tri.b.y + tri.c.y) / 3.0f};
        float mass = triangle_area(tri.a, tri.b, tri.c);
        coms.push_back(com);
        masses.push_back(mass);
        mass_sum += mass;
    }
    ImVec2 tmp_weighted_sum{0, 0};
    for (int i = 0; i < coms.size(); ++i)
    {
        tmp_weighted_sum = ImVec2{tmp_weighted_sum.x + coms[i].x * masses[i], tmp_weighted_sum.y + coms[i].y * masses[i]};
    }
    ImVec2 COM{tmp_weighted_sum.x / mass_sum, tmp_weighted_sum.y / mass_sum};

    for (auto p = points.begin(); p != points.end(); p++)
    {
        *p = ImVec2{p->x - COM.x, p->y - COM.y};
    }
    triangles.clear();
    for (auto tri_id : triangles_id)
    {
        triangles.push_back(triangle_type{points[tri_id[0]], points[tri_id[1]], points[tri_id[2]]});
    }
}

void Canvas::Delaunay_triangulate()
{
    triangles.clear();

    // Determinate the super triangle
    float minX = points[0].x;
    float minY = points[0].y;
    float maxX = minX;
    float maxY = minY;

    for (std::size_t i = 0; i < points.Size; ++i)
    {
        if (points[i].x < minX)
            minX = points[i].x;
        if (points[i].y < minY)
            minY = points[i].y;
        if (points[i].x > maxX)
            maxX = points[i].x;
        if (points[i].y > maxY)
            maxY = points[i].y;
    }

    const float dx = maxX - minX;
    const float dy = maxY - minY;
    const float deltaMax = std::max(dx, dy);
    const float midx = (minX + maxX) / 2;
    const float midy = (minY + maxY) / 2;

    ImVec2 p1(midx - 20 * deltaMax, midy - deltaMax);
    ImVec2 p2(midx, midy + 20 * deltaMax);
    ImVec2 p3(midx + 20 * deltaMax, midy - deltaMax);
    // std::cout << p1.transpose() << std::endl;
    // std::cout << p2.transpose() << std::endl;
    // std::cout << p3.transpose() << std::endl;

    auto almost_equal = [](float x, float y, int ulp = 2)
    {
        return fabs(x - y) < 1e-6;
    };

    auto almost_equal_v = [&](const ImVec2 &v1, const ImVec2 &v2)
    {
        return almost_equal(v1.x, v2.x) && almost_equal(v1.y, v2.y);
    };

    auto almost_equal_e = [&](const edge_type &e1, const edge_type &e2)
    {
        return (almost_equal_v(e1.v, e2.v) && almost_equal_v(e1.w, e2.w)) || (almost_equal_v(e1.v, e2.w) && almost_equal_v(e1.w, e2.v));
    };

    triangles.push_back(triangle_type{p1, p2, p3});
    for (auto p : points)
    {
        // printf("\nnew vert phase\n");
        std::vector<edge_type> polygon;

        for (auto t = triangles.begin(); t != triangles.end(); ++t)
        {
            // printf("[%f, %f\n%f, %f\n%f, %f]\n", t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y);
            if (t->circumCircleContains(p))
            {
                // printf("jji\n");
                t->isBad = true;
                polygon.push_back(edge_type{t->a, t->b});
                polygon.push_back(edge_type{t->b, t->c});
                polygon.push_back(edge_type{t->c, t->a});
            }
        }
        // printf("polygon length=%d\n", polygon.size());

        triangles.erase(std::remove_if(triangles.begin(), triangles.end(), [](triangle_type t)
                                       { return t.isBad; }),
                        triangles.end());

        for (auto e1 = polygon.begin(); e1 != polygon.end(); ++e1)
        {
            for (auto e2 = e1 + 1; e2 != polygon.end(); ++e2)
            {
                if (almost_equal_e(*e1, *e2))
                {
                    e1->isBad = true;
                    e2->isBad = true;
                }
            }
        }

        polygon.erase(std::remove_if(polygon.begin(), polygon.end(), [](edge_type &e)
                                     { return e.isBad; }),
                      polygon.end());
        // printf("polygon length=%d\n", polygon.size());

        for (auto e : polygon)
            triangles.push_back(triangle_type{e.v, e.w, p});
    }

    triangles.erase(std::remove_if(triangles.begin(), triangles.end(), [p1, p2, p3](triangle_type &t)
                                   { return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3); }),
                    end(triangles));

    triangles_id.clear();
    for (auto t : triangles)
    {
        glm::ivec3 new_tri_id;
        for (int i = 0; i < points.Size; ++i)
        {
            if (almost_equal_v(t.a, points[i]))
            {
                new_tri_id[0] = i;
            }
            if (almost_equal_v(t.b, points[i]))
            {
                new_tri_id[1] = i;
            }
            if (almost_equal_v(t.c, points[i]))
            {
                new_tri_id[2] = i;
            }
        }
        printf("(%d, %d, %d)\n", new_tri_id[0], new_tri_id[1], new_tri_id[2]);
        triangles_id.push_back(new_tri_id);
    }
    auto next_p_id = [this](int x)
    {
        return x == points.Size - 1 ? 0 : x + 1;
    };
    auto prev_p_id = [this](int x)
    {
        return x == 0 ? points.Size - 1 : x - 1;
    };
    std::vector<glm::ivec3>
        new_triangles_id;
    for (auto tri_id : triangles_id)
    {
        printf("tata\n");
        // if (!in_edges_range(points[tri_id[0]], points[prev_p_id(tri_id[0])], points[next_p_id(tri_id[0])], points[tri_id[1]]) || !in_edges_range(points[tri_id[0]], points[prev_p_id(tri_id[0])], points[next_p_id(tri_id[0])], points[tri_id[2]]))
        if (!in_edges_range(points[tri_id[0]], points[prev_p_id(tri_id[0])], points[next_p_id(tri_id[0])], points[tri_id[1]], points[tri_id[2]]))
        {
            printf("1\n");
            continue;
        }
        if (!in_edges_range(points[tri_id[1]], points[prev_p_id(tri_id[1])], points[next_p_id(tri_id[1])], points[tri_id[0]], points[tri_id[2]]))
        {
            printf("2\n");
            continue;
        }
        if (!in_edges_range(points[tri_id[2]], points[prev_p_id(tri_id[2])], points[next_p_id(tri_id[2])], points[tri_id[0]], points[tri_id[1]]))
        {
            printf("3\n");
            continue;
        }
        new_triangles_id.push_back(tri_id);
    }
    triangles.clear();
    for (auto tri_id : new_triangles_id)
    {
        triangles.push_back(triangle_type{points[tri_id[0]], points[tri_id[1]], points[tri_id[2]]});
    }
    // return triangles;
}

void Canvasrender()
{

    static bool show_save_file_modal = false;
    // if (show_file_window)
    // ShowFileWindow(&show_file_window);
    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
    // ImGui::SetNextWindowSize(ImVec2{400, 400});

    ImGui::Begin("Example: jini rendering", NULL, flags);

    // ImGui::Open
    // if (ImGui::Button("Save"))
    if (show_save_file_modal)
        ImGui::OpenPopup("Save file");
    IMGUI_DEMO_MARKER_("Examples/Custom Rendering");
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            IMGUI_DEMO_MARKER_("Examples/Menu");
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New"))
            {
            }
            // if (ImGui::MenuItem("Save", "Ctrl+S", &show_file_window))
            if (ImGui::MenuItem("Save", "Ctrl+S", &show_save_file_modal))
            {
                // show_file_window = true;if (ImGui::Button("Delete.."))
                // ImGui::OpenPopup("Delete?");
            }
            // if (ImGui::MenuItem("Save As.."))
            // {
            // }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginPopupModal("Save file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!");
        // ImGui::Separator();

        // static int unused_i = 0;
        // ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

        static bool save_to_default_folder = true;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        // ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
        ImGui::PopStyleVar();
        static char buf1[60] = "";
        ImGui::InputText("##", buf1, 60);
        ImGui::SameLine();
        ImGui::Checkbox("Default folder", &save_to_default_folder);
        // ImGui::Checkbox("Default folder", &save_to_default_folder);
        if (save_to_default_folder)
            // ImGui::Dummy(ImVec2(120, 15));
            ImGui::TextDisabled("The file will be save to ../assets/.");
        else
            ImGui::TextDisabled("Now input the absolute path.");
        ImGui::Separator();

        if (ImGui::Button("Save", ImVec2(120, 0)))
        {
            show_save_file_modal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            show_save_file_modal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
    // overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your
    // types and ImVec2/ImVec4. Dear ImGui defines overloaded operators but they are internal to imgui.cpp and not
    // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

    static ImVector<ImVec2> points;
    static ImVec2 scrolling(0.0f, 0.0f);
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    static bool finish_peri = false;

    ImGui::Checkbox("Enable grid", &opt_enable_grid);
    ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
    ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
    static ImVec2 mouse_canvas_pos_grid(0, 0);
    ImGui::Text("at (%.3f, %.3f)", mouse_canvas_pos_grid.x, mouse_canvas_pos_grid.y);

    // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
    // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
    // To use a child window instead we could use, e.g:
    //      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
    //      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
    //      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border, ImGuiWindowFlags_NoMove);
    //      ImGui::PopStyleColor();
    //      ImGui::PopStyleVar();
    //      [...]
    //      ImGui::EndChild();

    // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail(); // Resize canvas to what's available
    if (canvas_sz.x < 50.0f)
        canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f)
        canvas_sz.y = 50.0f;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    // Draw border and background color
    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_hovered = ImGui::IsItemHovered();                            // Hovered
    const bool is_active = ImGui::IsItemActive();                              // Held
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    // const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x - canvas_sz.x / 2, io.MousePos.y - origin.y - canvas_sz.y / 2);
    // ImGui::Text("at (%.3f, %.3f)", mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);
    // ImGui::Text("at (");
    mouse_canvas_pos_grid = ImVec2(mouse_pos_in_canvas.x / 64.0f, mouse_pos_in_canvas.y / 64.0f);

    float mouse_beginning_distance = 999.0f;
    if (points.Size > 0 && !finish_peri)
        mouse_beginning_distance = sqrtf((points[0].x - mouse_pos_in_canvas.x) * (points[0].x - mouse_pos_in_canvas.x) + (points[0].y - mouse_pos_in_canvas.y) * (points[0].y - mouse_pos_in_canvas.y));

    // Add first and second point
    if (is_hovered && !finish_peri && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (mouse_beginning_distance < 10.0f)
        {
            finish_peri = true;
        }
        else
            points.push_back(mouse_pos_in_canvas);
    }
    // if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    // {
    //     points.push_back(mouse_pos_in_canvas);
    //     points.push_back(mouse_pos_in_canvas);
    //     adding_line = true;
    // }
    // if (adding_line)
    // {
    //     points.back() = mouse_pos_in_canvas;
    //     if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
    //         adding_line = false;
    // }

    // Pan (we use a zero mouse threshold when there's no context menu)
    // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
    const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    if (ImGui::BeginPopup("context"))
    {
        // if (adding_line)
        //     points.resize(points.size() - 2);
        // adding_line = false;
        if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0))
        {
            if (finish_peri)
                finish_peri = false;
            else
                points.resize(points.size() - 2);
        }
        if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0))
        {
            finish_peri = false;
            points.clear();
        }
        ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    // if (opt_enable_grid)
    // {
    //     const float GRID_STEP = 64.0f;
    //     for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
    //         draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
    //     for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
    //         draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
    // }
    if (opt_enable_grid)
    {
        const float GRID_STEP = 64.0f;
        float canvas_x_remain = fmodf(canvas_sz.x, 2 * GRID_STEP);
        float canvas_y_remain = fmodf(canvas_sz.y, 2 * GRID_STEP);
        int x_grid_num_oneside = int(canvas_sz.x * 0.5 / GRID_STEP);
        int y_grid_num_oneside = int(canvas_sz.y * 0.5 / GRID_STEP);
        float x_grid_id = nearest_number_cell((canvas_p0.x + fmodf(scrolling.x, GRID_STEP) + canvas_x_remain / 2 - GRID_STEP - origin.x - canvas_sz.x / 2) / GRID_STEP, 1.0f);
        float y_grid_id = nearest_number_cell((canvas_p0.y + fmodf(scrolling.y, GRID_STEP) + canvas_y_remain / 2 - GRID_STEP - origin.y - canvas_sz.y / 2) / GRID_STEP, 1.0f);
        for (float x = fmodf(scrolling.x, GRID_STEP) + canvas_x_remain / 2 - GRID_STEP; x < canvas_sz.x; x += GRID_STEP)
        {
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
            // std::string coorinate_str = to_string_with_precision(((canvas_p0.x + x - origin.x - canvas_sz.x / 2) / GRID_STEP), 1);
            // std::string coorinate_str = to_string_with_precision((x_grid_id - (origin.x + canvas_sz.x / 2) / GRID_STEP), 1);
            if (!(origin.x + canvas_sz.x / 2 - canvas_p0.x - x) < 1e-2f)
            {
                std::string coorinate_str = to_string_with_precision((x_grid_id), 1);
                draw_list->AddText(ImVec2(canvas_p0.x + x, origin.y + canvas_sz.y / 2), IM_COL32(255, 255, 255, 255), coorinate_str.c_str());
            }
            x_grid_id++;
        }
        draw_list->AddLine(ImVec2(origin.x + canvas_sz.x / 2, canvas_p0.y), ImVec2(origin.x + canvas_sz.x / 2, canvas_p1.y), IM_COL32(200, 200, 200, 255));
        for (float y = fmodf(scrolling.y, GRID_STEP) + canvas_y_remain / 2 - GRID_STEP; y < canvas_sz.y; y += GRID_STEP)
        {
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
            if (!(origin.y + canvas_sz.y / 2 - canvas_p0.y - y) < 1e-2f)
            {
                std::string coorinate_str = to_string_with_precision((y_grid_id), 1);
                draw_list->AddText(ImVec2(origin.x + canvas_sz.x / 2, canvas_p0.y + y), IM_COL32(255, 255, 255, 255), coorinate_str.c_str());
            }
            y_grid_id++;
        }
        draw_list->AddLine(ImVec2(canvas_p0.x, origin.y + canvas_sz.y / 2), ImVec2(canvas_p0.x, origin.y + canvas_sz.y / 2), IM_COL32(200, 200, 200, 255));
        draw_list->AddCircleFilled(ImVec2(origin.x + canvas_sz.x / 2, origin.y + canvas_sz.y / 2), 3.0f, IM_COL32(155, 0, 0, 255), 64);
    }
    // for (int n = 0; n < points.Size; n += 2)
    //     draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
    if (points.Size > 1)
    {
        for (int n = 0; n < points.Size - 1; n++)
        {
            draw_list->AddLine(ImVec2(origin.x + points[n].x + canvas_sz.x / 2, origin.y + points[n].y + canvas_sz.y / 2), ImVec2(origin.x + points[n + 1].x + canvas_sz.x / 2, origin.y + points[n + 1].y + canvas_sz.y / 2), IM_COL32(255, 255, 0, 255), 2.0f);
        }
    }
    if (points.Size > 0 && !finish_peri)
    {
        draw_list->AddLine(ImVec2(origin.x + points[points.Size - 1].x + canvas_sz.x / 2, origin.y + points[points.Size - 1].y + canvas_sz.y / 2), ImVec2(origin.x + mouse_pos_in_canvas.x + canvas_sz.x / 2, origin.y + mouse_pos_in_canvas.y + canvas_sz.y / 2), IM_COL32(255, 255, 0, 255), 2.0f);
    }
    if (points.Size > 0 && finish_peri)
    {
        draw_list->AddLine(ImVec2(origin.x + points[points.Size - 1].x + canvas_sz.x / 2, origin.y + points[points.Size - 1].y + canvas_sz.y / 2), ImVec2(origin.x + points[0].x + canvas_sz.x / 2, origin.y + points[0].y + canvas_sz.y / 2), IM_COL32(255, 255, 0, 255), 2.0f);
    }
    draw_list->PopClipRect();
    if (points.Size > 0 && !finish_peri)
    {
        if (mouse_beginning_distance < 10.0f)
        {
            draw_list->AddCircleFilled(ImVec2(origin.x + points[0].x + canvas_sz.x / 2, origin.y + points[0].y + canvas_sz.y / 2), 6.0f, IM_COL32(255, 0, 0, 255), 64);
        }
        else
        {
            draw_list->AddCircleFilled(ImVec2(origin.x + points[0].x + canvas_sz.x / 2, origin.y + points[0].y + canvas_sz.y / 2), 3.0f, IM_COL32(155, 0, 0, 255), 64);
        }
    }

    // ImGui::EndTabItem();

    // ImGui::PopItemWidth();
    ImGui::End();
}

void Canvas::render()
{

    static bool show_save_file_modal = false;
    // if (show_file_window)
    // ShowFileWindow(&show_file_window);
    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
    // ImGui::SetNextWindowSize(ImVec2{400, 400});

    ImGui::Begin("Example: jini rendering", NULL, flags);

    // ImGui::Open
    // if (ImGui::Button("Save"))
    if (show_save_file_modal)
        ImGui::OpenPopup("Save file");
    IMGUI_DEMO_MARKER_("Examples/Custom Rendering");
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            IMGUI_DEMO_MARKER_("Examples/Menu");
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New"))
            {
            }
            // if (ImGui::MenuItem("Save", "Ctrl+S", &show_file_window))
            if (ImGui::MenuItem("Save", "Ctrl+S", &show_save_file_modal, finish_peri))
            {
                // show_file_window = true;if (ImGui::Button("Delete.."))
                // ImGui::OpenPopup("Delete?");
            }
            // if (ImGui::MenuItem("Save As.."))
            // {
            // }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginPopupModal("Save file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!");
        // ImGui::Separator();

        // static int unused_i = 0;
        // ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

        static bool save_to_default_folder = true;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        // ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
        ImGui::PopStyleVar();
        static char buf1[60] = "";
        ImGui::InputText(".obj", buf1, 60);
        ImGui::SameLine();
        ImGui::Checkbox("Default folder", &save_to_default_folder);
        // ImGui::Checkbox("Default folder", &save_to_default_folder);
        if (save_to_default_folder)
            // ImGui::Dummy(ImVec2(120, 15));
            ImGui::TextDisabled("The file will be save to ./assets/.");
        else
            ImGui::TextDisabled("Now input the absolute path.");
        ImGui::Separator();

        if (ImGui::Button("Save", ImVec2(120, 0)))
        {
            save_points("assets/" + std::string(buf1) + ".obj");
            show_save_file_modal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            show_save_file_modal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
    // overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your
    // types and ImVec2/ImVec4. Dear ImGui defines overloaded operators but they are internal to imgui.cpp and not
    // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

    // static ImVector<ImVec2> points;
    static ImVec2 scrolling(0.0f, 0.0f);
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    // static bool finish_peri = false;

    ImGui::Checkbox("Enable grid", &opt_enable_grid);
    ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
    ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
    static ImVec2 mouse_canvas_pos_grid(0, 0);
    ImGui::Text("at (%.3f, %.3f)", mouse_canvas_pos_grid.x, mouse_canvas_pos_grid.y);
    // ImGui::Text("tri (%d)", triangles.size());

    const char *items[] = {"segment", "Bézier"};
    static int item_current_idx = 0; // Here we store our selection data as an index.

    // Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
    const char *combo_preview_value = items[item_current_idx];

    if (ImGui::BeginCombo("line type", combo_preview_value, 0))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(items[n], is_selected))
                item_current_idx = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!finish_peri)
        ImGui::BeginDisabled();
    if (ImGui::Button("reset COM to origin"))
    {
        reset_COM_to_origin();
    }
    if (!finish_peri)
        ImGui::EndDisabled();
    // ImGui::Text("at (%.d)", points.Size);
    // ImGui::Text("finish_peri (%.d)", finish_peri);

    // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
    // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
    // To use a child window instead we could use, e.g:
    //      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
    //      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
    //      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border, ImGuiWindowFlags_NoMove);
    //      ImGui::PopStyleColor();
    //      ImGui::PopStyleVar();
    //      [...]
    //      ImGui::EndChild();

    // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail(); // Resize canvas to what's available
    if (canvas_sz.x < 50.0f)
        canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f)
        canvas_sz.y = 50.0f;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    // Draw border and background color
    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_hovered = ImGui::IsItemHovered();                            // Hovered
    const bool is_active = ImGui::IsItemActive();                              // Held
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    // const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
    // const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x - canvas_sz.x / 2, io.MousePos.y - origin.y - canvas_sz.y / 2);
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x - canvas_sz.x / 2, -(io.MousePos.y - origin.y - canvas_sz.y / 2));
    // ImGui::Text("at (%.3f, %.3f)", mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);
    // ImGui::Text("at (");
    mouse_canvas_pos_grid = ImVec2(mouse_pos_in_canvas.x / 64.0f, mouse_pos_in_canvas.y / 64.0f);

    float mouse_beginning_distance = 999.0f;
    if (points.Size > 0 && !finish_peri)
        mouse_beginning_distance = sqrtf((points[0].x - mouse_pos_in_canvas.x) * (points[0].x - mouse_pos_in_canvas.x) + (points[0].y - mouse_pos_in_canvas.y) * (points[0].y - mouse_pos_in_canvas.y));

    auto concept_to_canvas = [origin, canvas_sz](const ImVec2 &pos)
    {
        return ImVec2(origin.x + pos.x + canvas_sz.x / 2, origin.y - pos.y + canvas_sz.y / 2);
    };
    auto interset_edges = new_edge_intersect(mouse_pos_in_canvas);
    // std::vector<int> interset_edges;
    // Add point
    if (is_hovered && !finish_peri && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (mouse_beginning_distance < 10.0f)
        {
            finish_peri = true;
            // Delaunay_triangulate();
            triangulate();
        }
        else
        {
            if (interset_edges.size() == 0 || item_current_idx == 1)
                points.push_back(mouse_pos_in_canvas);
        }
    }

    // Pan (we use a zero mouse threshold when there's no context menu)
    // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
    const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    if (ImGui::BeginPopup("context"))
    {
        // if (adding_line)
        //     points.resize(points.size() - 2);
        // adding_line = false;
        if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0))
        {
            if (finish_peri)
            {
                finish_peri = false;
                triangles.clear();
            }
            else
                points.resize(points.size() - 1);
        }
        if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0))
        {
            finish_peri = false;
            points.clear();
            triangles.clear();
        }
        ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    // if (opt_enable_grid)
    // {
    //     const float GRID_STEP = 64.0f;
    //     for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
    //         draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
    //     for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
    //         draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
    // }
    if (opt_enable_grid)
    {
        const float GRID_STEP = 64.0f;
        float canvas_x_remain = fmodf(canvas_sz.x, 2 * GRID_STEP);
        float canvas_y_remain = fmodf(canvas_sz.y, 2 * GRID_STEP);
        int x_grid_num_oneside = int(canvas_sz.x * 0.5 / GRID_STEP);
        int y_grid_num_oneside = int(canvas_sz.y * 0.5 / GRID_STEP);
        float x_grid_id = nearest_number_cell((canvas_p0.x + fmodf(scrolling.x, GRID_STEP) + canvas_x_remain / 2 - GRID_STEP - origin.x - canvas_sz.x / 2) / GRID_STEP, 1.0f);
        float y_grid_id = nearest_number_cell((canvas_p0.y + fmodf(scrolling.y, GRID_STEP) + canvas_y_remain / 2 - GRID_STEP - origin.y - canvas_sz.y / 2) / GRID_STEP, 1.0f);
        for (float x = fmodf(scrolling.x, GRID_STEP) + canvas_x_remain / 2 - GRID_STEP; x < canvas_sz.x; x += GRID_STEP)
        {
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
            // std::string coorinate_str = to_string_with_precision(((canvas_p0.x + x - origin.x - canvas_sz.x / 2) / GRID_STEP), 1);
            // std::string coorinate_str = to_string_with_precision((x_grid_id - (origin.x + canvas_sz.x / 2) / GRID_STEP), 1);
            if (!(origin.x + canvas_sz.x / 2 - canvas_p0.x - x) < 1e-2f)
            {
                std::string coorinate_str = to_string_with_precision((x_grid_id), 1);
                draw_list->AddText(ImVec2(canvas_p0.x + x, origin.y + canvas_sz.y / 2), IM_COL32(255, 255, 255, 255), coorinate_str.c_str());
            }
            x_grid_id++;
        }
        draw_list->AddLine(ImVec2(origin.x + canvas_sz.x / 2, canvas_p0.y), ImVec2(origin.x + canvas_sz.x / 2, canvas_p1.y), IM_COL32(200, 200, 200, 255));
        for (float y = fmodf(scrolling.y, GRID_STEP) + canvas_y_remain / 2 - GRID_STEP; y < canvas_sz.y; y += GRID_STEP)
        {
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
            if (!(origin.y + canvas_sz.y / 2 - canvas_p0.y - y) < 1e-2f)
            {
                std::string coorinate_str = to_string_with_precision((-y_grid_id), 1);
                draw_list->AddText(ImVec2(origin.x + canvas_sz.x / 2, canvas_p0.y + y), IM_COL32(255, 255, 255, 255), coorinate_str.c_str());
            }
            y_grid_id++;
        }
        draw_list->AddLine(ImVec2(canvas_p0.x, origin.y + canvas_sz.y / 2), ImVec2(canvas_p0.x, origin.y + canvas_sz.y / 2), IM_COL32(200, 200, 200, 255));
        draw_list->AddCircleFilled(ImVec2(origin.x + canvas_sz.x / 2, origin.y + canvas_sz.y / 2), 3.0f, IM_COL32(155, 0, 0, 255), 64);
    }
    // for (int n = 0; n < points.Size; n += 2)
    //     draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
    if (item_current_idx == 0)
    {
        if (points.Size > 1 && !finish_peri)
        {
            // std::vector<bool> interset_marker(interset_edges.size(), false);
            std::vector<bool> interset_marker(points.Size - 1, false);
            for (auto ie : interset_edges)
                interset_marker[ie] = true;
            for (int n = 0; n < points.Size - 1; n++)
            {
                // concept_to_canvas
                if (interset_marker[n])
                    draw_list->AddLine(concept_to_canvas(points[n]), concept_to_canvas(points[n + 1]), IM_COL32(255, 0, 0, 255), 2.0f);
                else
                    draw_list->AddLine(concept_to_canvas(points[n]), concept_to_canvas(points[n + 1]), IM_COL32(255, 255, 0, 255), 2.0f);
            }
        }
        if (points.Size > 0 && !finish_peri)
        {
            draw_list->AddLine(concept_to_canvas(points[points.Size - 1]), concept_to_canvas(mouse_pos_in_canvas), IM_COL32(255, 255, 0, 255), 2.0f);
        }
        if (points.Size > 0 && finish_peri)
        {
            for (int n = 0; n < points.Size - 1; n++)
            {
                draw_list->AddLine(concept_to_canvas(points[n]), concept_to_canvas(points[n + 1]), IM_COL32(255, 255, 0, 255), 2.0f);
            }
            draw_list->AddLine(concept_to_canvas(points[points.Size - 1]), concept_to_canvas(points[0]), IM_COL32(255, 255, 0, 255), 2.0f);
            for (auto t : triangles)
                draw_list->AddTriangleFilled(concept_to_canvas(t.a), concept_to_canvas(t.b), concept_to_canvas(t.c), IM_COL32(255, 255, 0, 155));
        }
    }

    if (item_current_idx == 1)
    {
        int m = 50; // density
        int i, j;
        float t;
        // int C[100 - 1]; // 二项式系数数组。这样定义有些浪费空间，但不知道更好的办法
        // 算出来的曲线轨迹点x坐标。用float型相比int型曲线会更平滑
        float Bézier_curve_pointx;
        float Bézier_curve_pointy;

        auto C = Culculate_Binomial_Coefficient(points.Size - 1);
        // glLineStipple(1, 0xffff); // 设置为实线
        // glBegin(GL_LINE_STRIP);
        // 这里实际上是画了m条短直线来接近贝塞尔曲线
        ImVector<ImVec2> Bézier_curve_points;
        for (i = 0; i <= m; i++)
        {
            t = (float)i / (float)m;
            Bézier_curve_pointx = 0;
            Bézier_curve_pointy = 0;
            for (j = 0; j < points.Size; j++)
            {
                Bézier_curve_pointx += C[j] * pow(1 - t, points.Size - j - 1) * pow(t, j) * points[j].x;
                Bézier_curve_pointy += C[j] * pow(1 - t, points.Size - j - 1) * pow(t, j) * points[j].y;
            }
            // glVertex2f(Bézier_curve_pointx, Bézier_curve_pointy);
            Bézier_curve_points.push_back(ImVec2{Bézier_curve_pointx, Bézier_curve_pointy});
        }
        for (i = 0; i < m; i++)
        {
            draw_list->AddLine(concept_to_canvas(Bézier_curve_points[i]), concept_to_canvas(Bézier_curve_points[i + 1]), IM_COL32(255, 255, 0, 255), 2.0f);
        }
        // glEnd();
    }

    draw_list->PopClipRect();
    if (points.Size > 0 && !finish_peri)
    {
        if (mouse_beginning_distance < 10.0f)
        {
            draw_list->AddCircleFilled(concept_to_canvas(points[0]), 6.0f, IM_COL32(255, 0, 0, 255), 64);
            // draw_list->AddCircleFilled(ImVec2(origin.x + points[0].x + canvas_sz.x / 2, origin.y + points[0].y + canvas_sz.y / 2), 6.0f, IM_COL32(255, 0, 0, 255), 64);
        }
        else
        {
            draw_list->AddCircleFilled(concept_to_canvas(points[0]), 3.0f, IM_COL32(155, 0, 0, 255), 64);
            // draw_list->AddCircleFilled(ImVec2(origin.x + points[0].x + canvas_sz.x / 2, origin.y + points[0].y + canvas_sz.y / 2), 3.0f, IM_COL32(155, 0, 0, 255), 64);
        }
    }

    // ImGui::EndTabItem();

    // ImGui::PopItemWidth();
    ImGui::End();
}