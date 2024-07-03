include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubyqtuitools5_LIBRARY_SRC qtuitools.cpp qtuitoolshandlers.cpp)
add_library(rubyqtuitools5 MODULE ${rubyqtuitools5_LIBRARY_SRC})
target_link_libraries(rubyqtuitools5
    Qt5::UiTools
    smokeqtuitools
    qtruby5shared
)
target_compile_options(rubyqtuitools5 PRIVATE -Wall)
set_target_properties(rubyqtuitools5 PROPERTIES PREFIX "" OUTPUT_NAME qtuitools5)
install(TARGETS rubyqtuitools5 DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtuitools5.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtuitools5)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtuitools5.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtuitools5.bundle\" )" )
endif (Q_WS_MAC)
