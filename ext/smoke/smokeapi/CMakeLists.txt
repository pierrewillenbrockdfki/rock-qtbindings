project(smokeapi)

find_package(Qt5 REQUIRED COMPONENTS Core)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_executable(smoke5api main.cpp)
target_link_libraries(smoke5api smokeqtcore smokebase Qt5::Core)
target_compile_options(smoke5api PRIVATE -Wall)
install(    TARGETS smoke5api 
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
