cmake_minimum_required(VERSION 3.16)

project(MandelbrotExplorerQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64/lib/cmake/Qt6")
find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(MandelbrotExplorerQt
    main.cpp
    mandelbrotWidget.cpp
    mandelbrot.cpp
    coloring.cpp
)

target_link_libraries(MandelbrotExplorerQt Qt6::Widgets)
