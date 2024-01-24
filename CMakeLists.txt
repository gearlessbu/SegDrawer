cmake_minimum_required(VERSION 3.0.0)
project(my_opengl VERSION 0.1.0)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
add_executable(my_opengl 
main.cpp 
small_imgui/imgui.cpp
small_imgui/imgui_widgets.cpp
small_imgui/imgui_tables.cpp
small_imgui/imgui_impl_opengl3.cpp
small_imgui/imgui_impl_glfw.cpp
small_imgui/imgui_draw.cpp
small_imgui/imgui_demo.cpp
)
target_link_libraries(my_opengl  GL glfw)

add_executable(my_drawer 
drawer.cpp 
canvas.cpp
imgui_math.cpp
small_imgui/imgui.cpp
small_imgui/imgui_widgets.cpp
small_imgui/imgui_tables.cpp
small_imgui/imgui_impl_opengl3.cpp
small_imgui/imgui_impl_glfw.cpp
small_imgui/imgui_draw.cpp
small_imgui/imgui_demo.cpp
)
target_link_libraries(my_drawer  GL glfw)

# add_executable(dln_test 
# dln_test.cpp 
# canvas.cpp
# small_imgui/imgui.cpp
# small_imgui/imgui_widgets.cpp
# small_imgui/imgui_tables.cpp
# small_imgui/imgui_impl_opengl3.cpp
# small_imgui/imgui_impl_glfw.cpp
# small_imgui/imgui_draw.cpp
# small_imgui/imgui_demo.cpp
# )
# target_link_libraries(dln_test  GL glfw)
 