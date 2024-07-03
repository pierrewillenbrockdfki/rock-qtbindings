include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ruby/qtruby/src)

set(rubyqtdeclarative5_LIBRARY_SRC qtdeclarative.cpp qtdeclarativehandlers.cpp)
add_library(rubyqtdeclarative5 MODULE ${rubyqtdeclarative5_LIBRARY_SRC})
target_link_libraries(rubyqtdeclarative5
    Qt5::Qml Qt5::Quick
    smokeqtdeclarative
    qtruby5shared
)
target_compile_options(rubyqtdeclarative5 PRIVATE -Wall)
set_target_properties(rubyqtdeclarative5 PROPERTIES PREFIX "" OUTPUT_NAME qtdeclarative5)
install(TARGETS rubyqtdeclarative5 DESTINATION ${CUSTOM_RUBY_SITE_ARCH_DIR})
install(FILES qtdeclarative5.rb DESTINATION ${CUSTOM_RUBY_SITE_LIB_DIR}/qtdeclarative5)

if (Q_WS_MAC)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtdeclarative5.so\"  \"${CUSTOM_RUBY_SITE_ARCH_DIR}/qtdeclarative5.bundle\" )" )
endif (Q_WS_MAC)
