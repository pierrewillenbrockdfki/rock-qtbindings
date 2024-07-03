include_directories( ${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/smoke  )

set(QT_INCLUDE_DIRS_XML_LIST
    ${Qt5Core_INCLUDE_DIRS} ${Qt5WebKit_INCLUDE_DIRS})
list(TRANSFORM QT_INCLUDE_DIRS_XML_LIST REPLACE ".+" "<dir>\\0</dir>")
list(REMOVE_DUPLICATES QT_INCLUDE_DIRS_XML_LIST)
list(JOIN QT_INCLUDE_DIRS_XML_LIST "\n        " QT_INCLUDE_DIRS_XML)

configure_file(config.xml.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.xml @ONLY)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
	${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp

    COMMAND smokegen
    ARGS -config "${CMAKE_CURRENT_BINARY_DIR}/config.xml" -smokeconfig "${CMAKE_CURRENT_SOURCE_DIR}/smokeconfig.xml"
         -- ${CMAKE_CURRENT_SOURCE_DIR}/qtwebkit_includes.h
    DEPENDS smokegen generator_smoke smokeconfig.xml qtwebkit_includes.h "${CMAKE_CURRENT_BINARY_DIR}/config.xml" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

macro_add_file_dependencies( ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp )

include_directories (${CMAKE_SOURCE_DIR}/smoke ${CMAKE_SOURCE_DIR}/smoke/qtwebkit )

set(smokeqtwebkit_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
	${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp)

IF(CMAKE_CXX_FLAGS MATCHES "-fvisibility")
    ADD_DEFINITIONS(-DGCC_VISIBILITY)
ENDIF(CMAKE_CXX_FLAGS MATCHES "-fvisibility")
add_library(smokeqtwebkit SHARED ${smokeqtwebkit_LIB_SRCS})

target_link_libraries(  smokeqtwebkit 
                        smokeqtcore 
                        smokeqtgui 
                        smokeqtnetwork
                        Qt5::Core
                        Qt5::Gui
                        Qt5::Network
                        Qt5::WebKit )
target_compile_options(smokeqtwebkit PRIVATE -Wall)

set_target_properties(smokeqtwebkit PROPERTIES VERSION 3.0.0 SOVERSION 3 )

install(TARGETS smokeqtwebkit DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} )
install(FILES ${CMAKE_SOURCE_DIR}/smoke/qtwebkit_smoke.h DESTINATION include/smoke )
