include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubyqtwebkit_LIBRARY_SRC qtwebkit.cpp qtwebkithandlers.cpp)
add_library(rubyqtwebkit MODULE ${rubyqtwebkit_LIBRARY_SRC})
target_link_libraries(rubyqtwebkit
    Qt5::WebKit Qt5::WebKitWidgets
    smokeqtwebkit
    qtruby4shared
)
target_compile_options(rubyqtwebkit PRIVATE -Wall)
set_target_properties(rubyqtwebkit PROPERTIES PREFIX "" OUTPUT_NAME qtwebkit)
install(TARGETS rubyqtwebkit DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtwebkit.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtwebkit)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkit.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtwebkit.bundle\" )" )
endif (Q_WS_MAC)
