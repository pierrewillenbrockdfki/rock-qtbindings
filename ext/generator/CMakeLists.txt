project(smokegenerator)

find_package(Qt5 REQUIRED COMPONENTS Core Xml)

find_package(Clang REQUIRED)
find_package(LLVM REQUIRED)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if (NOT LIB_INSTALL_DIR)
    set (LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
endif (NOT LIB_INSTALL_DIR)

include_directories(${Qt5Core_INCLUDE_DIRS} parser)

set(generator_SRC
    main.cpp
    clanggeneratorvisitor.cpp
    options.cpp
    type.cpp
)

# force RPATH so that the binary is usable from within the build tree
set (CMAKE_SKIP_BUILD_RPATH FALSE)
set (CMAKE_SKIP_RPATH FALSE)

add_executable(smokegen ${generator_SRC})
target_link_libraries(smokegen
    Qt5::Core Qt5::Xml
    clang-cpp LLVM
    )
set_target_properties(smokegen PROPERTIES COMPILE_DEFINITIONS __GENERATOR_BUILDING ENABLE_EXPORTS TRUE)
target_compile_options(smokegen PRIVATE -fPIC -Wall)
target_compile_definitions(smokegen PRIVATE QT_NO_KEYWORDS)
target_include_directories(smokegen PRIVATE ${CLANG_INCLUDE_DIRS})

install(TARGETS smokegen DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES generator_export.h options.h type.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/smokegen)

add_subdirectory(generators)
