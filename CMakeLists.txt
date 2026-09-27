cmake_minimum_required(VERSION 3.16)
project(VirtualPetridish LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)

find_library(OPENCL_LIB OpenCL)
if(NOT OPENCL_LIB)
    message(FATAL_ERROR "libOpenCL.so not found - install ocl-icd-opencl-dev")
endif()

set(CPM_DIR ${CMAKE_SOURCE_DIR}/CPM)

add_executable(CPM
    ${CPM_DIR}/ActivityContraint.cpp
    ${CPM_DIR}/AdhesionConstraint.cpp
    ${CPM_DIR}/Cell.cpp
    ${CPM_DIR}/CellularPotts.cpp
    ${CPM_DIR}/DiceSet.cpp
    ${CPM_DIR}/Display.cpp
    ${CPM_DIR}/DisplayHelper.cpp
    ${CPM_DIR}/Grid.cpp
    ${CPM_DIR}/GridManadger.cpp
    ${CPM_DIR}/HamiltonianConstraint.cpp
    ${CPM_DIR}/imgui.cpp
    ${CPM_DIR}/imgui_demo.cpp
    ${CPM_DIR}/imgui_draw.cpp
    ${CPM_DIR}/imgui_tables.cpp
    ${CPM_DIR}/imgui_widgets.cpp
    ${CPM_DIR}/imgui_impl_glfw.cpp
    ${CPM_DIR}/imgui_impl_opengl3.cpp
    ${CPM_DIR}/main.cpp
    ${CPM_DIR}/OpenCL.cpp
    ${CPM_DIR}/Parameters.cpp
    ${CPM_DIR}/PerimeterConstraint.cpp
    ${CPM_DIR}/PersistenceConstraint.cpp
    ${CPM_DIR}/PixelsByCell.cpp
    ${CPM_DIR}/Simulation.cpp
    ${CPM_DIR}/Statistics.cpp
    ${CPM_DIR}/VolumeConstraint.cpp
)

target_include_directories(CPM PRIVATE ${CPM_DIR} ${CPM_DIR}/include)

target_link_libraries(CPM PRIVATE
    OpenGL::GL
    PkgConfig::GLFW
    ${OPENCL_LIB}
    Threads::Threads
)

# OpenCL.cpp loads the kernel via a cwd-relative std::ifstream("kernel.cl"),
# so it must sit next to whatever binary actually gets launched.
add_custom_command(TARGET CPM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CPM_DIR}/kernel.cl
        $<TARGET_FILE_DIR:CPM>/kernel.cl
)
