project(smokedeptool)

find_package(Qt5 REQUIRED COMPONENTS Core)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_executable(smokedeptool main.cpp)
target_link_libraries(smokedeptool smokeqtcore Qt5::Core)
