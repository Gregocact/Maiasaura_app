cmake_minimum_required(VERSION 3.20)
project(MaiasauraTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MaiasauraTool
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/Track.h
    src/Scanner.h
    src/Scanner.cpp
    src/PlaylistModel.h
    src/PlaylistModel.cpp
)


target_link_libraries(MaiasauraTool PRIVATE Qt6::Widgets)