cmake_minimum_required(VERSION 3.16)
project(OpenGLApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Główne źródła
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Dodanie executabla (bez nagłówków)
add_executable(${PROJECT_NAME} ${SOURCES} "src/stb_image.h")

# GLFW
add_subdirectory(external/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# GLAD
add_library(glad STATIC external/glad/src/glad.c "src/stb_image.h")
target_include_directories(glad PUBLIC external/glad/include)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

# GLM
target_include_directories(${PROJECT_NAME} PUBLIC external/glm)

# IMGUI
set(IMGUI_DIR external/imgui)
target_sources(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Inne katalogi include (opcjonalne)
target_include_directories(${PROJECT_NAME} PRIVATE include external/glfw/include external/glad/include)