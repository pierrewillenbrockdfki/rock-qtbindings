IF(Qt5DBus_FOUND AND NOT WIN32)
    ADD_DEFINITIONS (-DQT_QTDBUS)
ENDIF(QT_QTDBUS_FOUND)

IF(QWT_FOUND)
    ADD_DEFINITIONS (-DQT_QWT)
ENDIF(QWT_FOUND)

if(RUBY_VERSION)
    ADD_DEFINITIONS (-DRUBY_VERSION=0x${RUBY_VERSION_NUMBER})
ENDIF(RUBY_VERSION)

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES (${QT_INCLUDES})


set(qtrubyshared_LIB_SRCS 
   Qt.cpp 
   handlers.cpp 
   marshall_types.cpp
   )

set(qtruby_LIB_SRCS qtruby.cpp)

# Use 'MODULE' instead of 'SHARED' so that qtruby will be built as a bundle on Mac OS X
# instead of a dyld. But is this correct (rjd)? It needs testing.
add_library(qtruby5shared SHARED ${qtrubyshared_LIB_SRCS})
add_library(qtruby5 MODULE ${qtruby_LIB_SRCS})

if(Qt5DBus_FOUND AND NOT WIN32)
  target_link_libraries(  qtruby5shared 
                              Qt5::Core 
                              Qt5::Gui 
                              Qt5::Widgets
                              ${RUBY_LIBRARY}
                              Qt5::Network
                              Qt5::PrintSupport
                              smokebase
                              smokeqtcore 
                              smokeqtgui 
                              smokeqtxml 
                              smokeqtopengl 
                              smokeqtsql 
                              smokeqtnetwork 
                              smokeqtdbus 
                              smokeqtsvg )

  target_link_libraries(  qtruby5
                              Qt5::Core 
                              Qt5::Gui 
                              Qt5::Widgets
                              Qt5::DBus
                              ${RUBY_LIBRARY} 
                              Qt5::Network 
                              smokeqtcore 
                              smokeqtgui 
                              smokeqtxml 
                              smokeqtopengl 
                              smokeqtsql 
                              smokeqtnetwork 
                              smokeqtdbus 
                              smokeqtsvg
                              qtruby5shared )
else(Qt5DBus_FOUND)
  target_link_libraries(  qtruby5shared 
                              Qt5::Core 
                              Qt5::Gui 
                              Qt5::Widgets
                              ${RUBY_LIBRARY}
                              Qt5::Network
                              Qt5::PrintSupport
                              smokebase
                              smokeqtcore 
                              smokeqtgui 
                              smokeqtxml 
                              smokeqtopengl 
                              smokeqtsql 
                              smokeqtnetwork 
                              smokeqtsvg )

  target_link_libraries(  qtruby5 
                              Qt5::Core 
                              Qt5::Gui 
                              Qt5::Widgets
                              ${RUBY_LIBRARY}
                              Qt5::Network 
                              smokeqtcore 
                              smokeqtgui 
                              smokeqtxml 
                              smokeqtopengl 
                              smokeqtsql 
                              smokeqtnetwork 
                              smokeqtsvg
                              qtruby5shared )
endif(Qt5DBus_FOUND)

set_target_properties(qtruby5 PROPERTIES PREFIX "")
set_target_properties(qtruby5shared PROPERTIES VERSION 2.0.0 SOVERSION 2)

install(TARGETS qtruby5shared DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} )
install(TARGETS qtruby5 DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})

install(FILES qtruby.h marshall.h marshall_basetypes.h marshall_complex.h marshall_macros.h marshall_primitives.h marshall_types.h smokeruby.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/qtruby)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtruby5.so\" \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtruby5.bundle\" )" )
endif (Q_WS_MAC)
