project(SMOKE)

add_definitions(-DSMOKE_BUILDING)

add_subdirectory(smokebase)
add_subdirectory(qtcore)
add_subdirectory(deptool)
add_subdirectory(smokeapi)

macro_optional_add_bindings(Qt5Network_FOUND "QtNetwork" qtnetwork)
if (NOT WIN32)
  macro_optional_add_bindings(Qt5DBus_FOUND "QtDBus" qtdbus)
endif (NOT WIN32)
macro_optional_add_bindings(Qt5Gui_FOUND "QtGui" qtgui)
macro_optional_add_bindings(Qt5Widgets_FOUND "QtWidgets" qtwidgets)
macro_optional_add_bindings(Qt5X11Extras_FOUND "QtX11Extras" qtx11extras)
macro_optional_add_bindings(Qt5Svg_FOUND "QtSvg" qtsvg)
macro_optional_add_bindings(Qt5Sql_FOUND "QtSql" qtsql)
macro_optional_add_bindings(Qt5Xml_FOUND "QtXml" qtxml)
macro_optional_add_bindings(Qt5OpenGL_FOUND "QtOpenGL" qtopengl)
macro_optional_add_bindings(Qt5WebKit_FOUND "QtWebKit" qtwebkit)
macro_optional_add_bindings(Qt5Script_FOUND "QtScript" qtscript)
macro_optional_add_bindings(Qt5UiTools_FOUND "QtUiTools" qtuitools)
macro_optional_add_bindings(Qt5Test_FOUND "QtTest" qttest)
macro_optional_add_bindings(Qt5Help_FOUND "QtHelp" qthelp)

# for qtguess.pl
if (APPLE)
set(qt_test_threshold 15)
else (APPLE)
set(qt_test_threshold 10)
endif (APPLE)

macro_display_bindings_log()

########### install files ###############

install( FILES  smoke.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
