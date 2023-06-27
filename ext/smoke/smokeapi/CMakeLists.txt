project(smokeapi)

find_package(Qt5 REQUIRED COMPONENTS Core)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_executable(smokeapi main.cpp)
target_link_libraries(smokeapi smokeqtcore smokebase Qt5::Core)
install(    TARGETS smokeapi 
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
