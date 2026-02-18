project(smokedeptool)

find_package(Qt5 REQUIRED COMPONENTS Core)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_executable(smoke5deptool main.cpp)
target_link_libraries(smoke5deptool smokeqtcore Qt5::Core)
target_compile_options(smoke5deptool PRIVATE -Wall)
