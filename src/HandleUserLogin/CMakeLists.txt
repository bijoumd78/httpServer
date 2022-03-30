include_directories(../logger)


file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(HandleUserLogin ${SOURCES} ${HEADERS})

if(WIN32)
    add_dependencies(HandleUserLogin logger)
endif()

target_link_libraries(HandleUserLogin PRIVATE logger)

set_target_properties(HandleUserLogin PROPERTIES FOLDER "HandleUserLogin")

install(TARGETS
    HandleUserLogin
    ARCHIVE
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
    RUNTIME
    DESTINATION ${INSTALL_BINDIR}
    COMPONENT bin
    LIBRARY
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
    PUBLIC_HEADER
    DESTINATION ${INSTALL_INCLUDEDIR}
    COMPONENT dev)