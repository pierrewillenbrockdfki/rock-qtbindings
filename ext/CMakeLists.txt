project(qtbind)

add_subdirectory(generator)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

include(MacroOptionalFindPackage)
include(MacroOptionalAddBindings)
include(MacroLogFeature)
include(CheckCXXSourceCompiles)

# Call cmake/modules/FindQt5.cmake
# We are using the version provided by the Github project Kitware/CMake
# This sets a bunch of QT_XXX variables used throughout the build process
FIND_PACKAGE (Qt5 REQUIRED COMPONENTS Core DBus Gui Help Network OpenGL PrintSupport Script Sql Svg Test UiTools WebKit WebKitWidgets Widgets X11Extras Xml)

add_definitions (${Qt5Core_DEFINITIONS} ${Qt5DBus_DEFINITIONS}
    ${Qt5Gui_DEFINITIONS} ${Qt5Help_DEFINITIONS} ${Qt5Network_DEFINITIONS}
    ${Qt5OpenGL_DEFINITIONS} ${Qt5Script_DEFINITIONS}
    ${Qt5Sql_DEFINITIONS} ${Qt5Svg_DEFINITIONS} ${Qt5Test_DEFINITIONS}
    ${Qt5UiTools_DEFINITIONS} ${Qt5WebKit_DEFINITIONS}
    ${Qt5Widgets_DEFINITIONS} ${Qt5Xml_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    ${Qt5Core_INCLUDE_DIRS} ${Qt5DBus_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Help_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Script_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS} ${Qt5Svg_INCLUDE_DIRS} ${Qt5Test_INCLUDE_DIRS}
    ${Qt5UiTools_INCLUDE_DIRS} ${Qt5WebKit_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS})

FIND_PACKAGE (Ruby REQUIRED)
FIND_PACKAGE (OpenGL REQUIRED)

add_subdirectory(smoke)
add_subdirectory(ruby)
