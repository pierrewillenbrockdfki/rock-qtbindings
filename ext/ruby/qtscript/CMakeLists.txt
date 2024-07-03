include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(qtscript5_LIBRARY_SRC qtscript.cpp qtscripthandlers.cpp)

add_library(qtscript5 MODULE ${qtscript5_LIBRARY_SRC})
target_link_libraries(qtscript5
    Qt5::Script
    smokeqtscript
    qtruby5shared
)
target_compile_options(qtscript5 PRIVATE -Wall)
set_target_properties(qtscript5 PROPERTIES PREFIX "")
install(TARGETS qtscript5 DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtscript5.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtscript5)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtscript5.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtscript5.bundle\" )" )
endif (Q_WS_MAC)

