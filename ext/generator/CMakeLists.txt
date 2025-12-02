project(smokegenerator)

find_package(Qt5 REQUIRED COMPONENTS Core Xml)

find_package(Clang REQUIRED)
find_package(LLVM REQUIRED)

execute_process(COMMAND clang -print-resource-dir
    OUTPUT_VARIABLE CLANG_RESOURCDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

execute_process(COMMAND clang++ -E -v -x c++ -
    OUTPUT_VARIABLE CLANG_VERBOSE_OUTPUT
    ERROR_VARIABLE CLANG_VERBOSE_ERROR
    INPUT_FILE /dev/null
    )

string(REGEX MATCHALL "[^\n\r]+" CLANG_VERBOSE_ERROR_LINES ${CLANG_VERBOSE_ERROR})
set(PARSE_STATE 0)
set(CLANG_BUILTIN_INCLUDE_DIRS)
foreach(LINE ${CLANG_VERBOSE_ERROR_LINES})
    if(${PARSE_STATE} EQUAL 0)
        if("${LINE}" STREQUAL "#include <...> search starts here:")
            set(PARSE_STATE 1)
        endif()
    elseif(${PARSE_STATE} EQUAL 1)
        if("${LINE}" STREQUAL "End of search list.")
            set(PARSE_STATE 2)
        else()
            string(STRIP "${LINE}" STRIPPED)
            list(APPEND CLANG_BUILTIN_INCLUDE_DIRS "${STRIPPED}")
        endif()
    else()
        # do nothing
    endif()
endforeach()

set(COMPILETIME_CLANG_ARGS
    "\"-std=c++${CMAKE_CXX_STANDARD_DEFAULT}\",")
set(COMPILETIME_CLANG_INCLUDES, )
foreach(INCLUDE ${CLANG_BUILTIN_INCLUDE_DIRS})
    set(COMPILETIME_CLANG_INCLUDES "${COMPILETIME_CLANG_INCLUDES}  \"${INCLUDE}\",\n")
endforeach()
set(COMPILETIME_CLANG_RESOURCEPATH "\"${CLANG_RESOURCDIR}\"")

configure_file(compiletime_settings.inc.in
    compiletime_settings.inc
    @ONLY
    )

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
target_include_directories(smokegen PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS smokegen DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES generator_export.h options.h type.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/smokegen)

add_subdirectory(generators)
