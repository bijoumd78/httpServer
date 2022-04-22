include_directories(../logger)


file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(RedisCache ${SOURCES} ${HEADERS})

if(WIN32)
    add_dependencies(RedisCache logger)
endif()

target_link_libraries(RedisCache PRIVATE logger)

set_target_properties(RedisCache PROPERTIES FOLDER "RedisCache")

install(TARGETS
    RedisCache
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