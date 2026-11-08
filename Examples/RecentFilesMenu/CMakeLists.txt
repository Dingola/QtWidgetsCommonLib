cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

project(RecentFilesMenuExample LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(QtWidgetsCommonLib CONFIG REQUIRED)

qt_standard_project_setup()

add_executable(RecentFilesMenuExample main.cpp)
target_compile_features(RecentFilesMenuExample PRIVATE cxx_std_20)
target_link_libraries(RecentFilesMenuExample PRIVATE Qt6::Widgets QtWidgetsCommonLib::QtWidgetsCommonLib)
