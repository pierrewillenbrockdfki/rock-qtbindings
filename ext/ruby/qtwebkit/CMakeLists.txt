include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubyqtwebkit5_LIBRARY_SRC qtwebkit.cpp qtwebkithandlers.cpp)
add_library(rubyqtwebkit5 MODULE ${rubyqtwebkit5_LIBRARY_SRC})
target_link_libraries(rubyqtwebkit5
    Qt5::WebKit Qt5::WebKitWidgets
    smokeqtwebkit
    qtruby5shared
)
target_compile_options(rubyqtwebkit5 PRIVATE -Wall)
set_target_properties(rubyqtwebkit5 PROPERTIES PREFIX "" OUTPUT_NAME qtwebkit5)
install(TARGETS rubyqtwebkit5 DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtwebkit5.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtwebkit5)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkit5.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkit5.bundle\" )" )
endif (Q_WS_MAC)
